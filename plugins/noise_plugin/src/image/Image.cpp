#include "Image.hpp"
#include <algorithm>
#include <xmmintrin.h>
#include <immintrin.h>
#include <fstream>
#include <iostream>

namespace cnoise {

    namespace img {
        inline void unpack_16bit(int16_t in, uint8_t* dest) {
            dest[0] = static_cast<uint8_t>(in & 0x00ff);
            dest[1] = static_cast<uint8_t>((in & 0xff00) >> 8);
            return;
        }

        inline void unpack_u16(const uint16_t& in, char* dest) {
            dest[0] = static_cast<char>(in & 0x00ff);
            dest[1] = static_cast<char>((in & 0xff00) >> 8); 
        }

        inline void unpack_32bit(int32_t integer, uint8_t* dest) {
            dest[0] = static_cast<uint8_t>((integer & 0x000000ff));
            dest[1] = static_cast<uint8_t>((integer & 0x0000ff00) >> 8);
            dest[2] = static_cast<uint8_t>((integer & 0x00ff0000) >> 16);
            dest[3] = static_cast<uint8_t>((integer & 0xff000000) >> 24);
            return;
        }

        inline void unpack_float(float in, uint8_t* dest) {
            uint8_t* bytes = reinterpret_cast<uint8_t*>(&in);
            dest[0] = *bytes++;
            dest[1] = *bytes++;
            dest[2] = *bytes++;
            dest[3] = *bytes++;
            return;
        }

        auto float_to_16 = [](const float& val) {
            return static_cast<uint16_t>(val);    
        };

        inline std::vector<float> NormalizeFloatData(const std::vector<float>& data) {
            auto min_max = std::minmax_element(data.cbegin(), data.cend());
            const float& min = *min_max.first;
            const float& max = *min_max.second;
            std::vector<float> result(data.cbegin(), data.cend());
            for(auto& elem : result) {
                elem = (elem - min) / (max - min);
            }
            return result;
        }

        inline std::vector<uint16_t> ToInt16(const std::vector<float>& data) {
            auto norm_data = NormalizeFloatData(data);
            std::vector<uint16_t> result; result.reserve(norm_data.size());
            for(size_t i = 0; i < norm_data.size(); ++i) {
                result.push_back(norm_data[i] * std::numeric_limits<uint16_t>::max());
            }
            return result;
        }

        ImageWriter::ImageWriter(int _width, int _height) : width(_width), height(_height) {
            rawData.reserve(width * height);
        }

        void ImageWriter::FreeMemory() {
            rawData.clear();
            rawData.shrink_to_fit();
            pixelData.clear();
            pixelData.shrink_to_fit();
        }

        void ImageWriter::WritePNG(const char * filename, int compression_level) {
            // We store values here
            std::vector<float> scaled_data;
            // Copy values over to pixelData, for a grayscale image.
            scaled_data.resize(rawData.size());
            __m128 scale, min, ratio;
            // Register used to scale up/down
            scale = _mm_set1_ps(255.0f);
            auto min_max = std::minmax_element(rawData.begin(), rawData.end());
            // register holding min element
            min = _mm_set1_ps(*min_max.first);
            // register used as divisor
            ratio = _mm_sub_ps(min, _mm_set1_ps(*min_max.second));

            for (size_t i = 0; i < rawData.size(); i += 4) {
                __m128 reg; // Will hold floats in this register.
                reg = _mm_load_ps1(&rawData[i]);
                // get "reg" into 0.0 - 1.0 scale.
                reg = _mm_sub_ps(reg, min);
                reg = _mm_div_ps(reg, ratio);
                // Multiply reg by scale.
                reg = _mm_mul_ps(reg, scale);
                // Store data in tmpBuffer.
                _mm_store1_ps(&scaled_data[i], reg);
            }

            pixelData.resize(scaled_data.size() * 4);
            for (size_t y = 0; y < height; ++y) {
                for (size_t x = 0; x < width; ++x) {
                    size_t idx = 4 * width * y + 4 * x;
                    pixelData[idx + 0] = scaled_data[width * y + x];
                    pixelData[idx + 1] = scaled_data[width * y + x];
                    pixelData[idx + 2] = scaled_data[width * y + x];
                    pixelData[idx + 3] = 255;
                }
            }
            if (compression_level == 0) {
                // Saves uncompressed image using "pixelData" to "filename"
                unsigned err;
                //err = lodepng::encode(filename, &pixelData[0], width, height);
                if (!err) {
                    return;
                }
                else {
                    //std::cout << "Error encoding image, code " << err << ": " << lodepng_error_text(err) << std::endl;
                }
            }
            else {
                // TODO: Implement compression. Need to study parameters of LodePNG more. See: https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_optimize_png.cpp
            }
            pixelData.clear();
            pixelData.shrink_to_fit();
        }

        void ImageWriter::WriteInt16(const char* filename) {
            const std::vector<uint16_t> file_data = ToInt16(rawData);
            std::ofstream out;
            out.open(filename, std::ios::out | std::ios::binary);
            if(!out.is_open()) {
                throw std::runtime_error("Failed to open file for writing!");
            }
            else {
                for(size_t i = 0; i < file_data.size(); ++i) {
                    const uint16_t& val = file_data[i];
                    char buff[2];
                    unpack_u16(val, buff);
                    out.write(buff, 2);
                }
                out.close();
            }
        }

        void ImageWriter::WriteRaw32(const char* filename) {
            std::ofstream out;
            out.open(filename, std::ios::out | std::ios::binary);
            for (size_t i = 0; i < rawData.size(); ++i) {
                float val = rawData[i];
                unsigned char buff[4];
                unpack_float(val, buff);
                out.write(reinterpret_cast<char*>(buff), 4);
            }
            out.close();
        }

        void ImageWriter::WriteTER(const char* filename) {

            // Output data size.
            size_t byte_width = width * sizeof(int16_t);
            size_t total_size = byte_width * height;

            // Buffer a line of data at a time.
            std::vector<uint8_t> lineBuffer;
            lineBuffer.reserve(byte_width);

            // Open output file stream
            std::ofstream out;
            out.clear();

            // Oopen output file in binary mode
            out.open(filename, std::ios::out | std::ios::binary);

            // Build header.
            // height_scale - 0.50f in divisor means 0.25m between sampling points.
            int16_t height_scale = static_cast<int16_t>(floorf(32768.0f / 30.0f));
            // Buffer used for unpacking various types into correct format for writing to stream.
            uint8_t buffer[4];
            out.write("TERRAGENTERRAIN ", 16); // First element of header.
            // Write terrain size.
            out.write("SIZE", 4);
            unpack_16bit(static_cast<int16_t>(std::min(width, height) - 1), buffer);
            out.write(reinterpret_cast<char*>(buffer), 2);
            out.write("\0\0", 2);
            // X dim.
            out.write("XPTS", 4);
            unpack_16bit(static_cast<int16_t>(width), buffer);
            out.write(reinterpret_cast<char*>(buffer), 2);
            out.write("\0\0", 2);
            // Y dim
            out.write("YPTS", 4);
            unpack_16bit(static_cast<int16_t>(height), buffer);
            out.write(reinterpret_cast<char*>(buffer), 2);
            out.write("\0\0", 2);
            // Write scale.
            out.write("SCAL", 4);
            // point-sampling scale is XYZ quantity, write same value three times.
            unpack_float(15.0f, buffer);
            out.write(reinterpret_cast<char*>(buffer), 4);
            out.write(reinterpret_cast<char*>(buffer), 4);
            out.write(reinterpret_cast<char*>(buffer), 4);
            // Write height scale
            out.write("ALTW", 4);
            unpack_16bit(height_scale, buffer);
            out.write(reinterpret_cast<char*>(buffer), 4);
            out.write("\0\0", 2);
            if (out.fail() || out.bad()) {
                throw;
            }

            // Build and write each horizontal line to the file.
            std::vector<int16_t> height_values = convertRawData_Ranged<int16_t>(rawData, 0.0f, 1000.0f);
            for (size_t i = 0; i < height_values.size(); ++i) {
                uint8_t buffer[2];
                unpack_16bit(height_values[i], buffer);
                out.write(reinterpret_cast<char*>(buffer), 2);
            }

            out.write("EOF", 4);
            // Close output file.
            out.close();
        }

        void ImageWriter::SetRawData(const std::vector<float>& raw) {
            rawData = raw;
        }

        std::vector<float> ImageWriter::GetRawData() const {
            return rawData;
        }
    }
}