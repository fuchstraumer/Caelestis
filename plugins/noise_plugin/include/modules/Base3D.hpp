		class Module3D {
			// Delete copy ctor and operator
			Module3D(const Module3D& other) = delete;
			Module3D& operator=(const Module3D& other) = delete;
			Module3D(Module3D&& other) = delete;
			Module3D& operator=(Module3D&& other) = delete;
		public:

			// Each Module3D must have a width and height, as this specifies the size of 
			// the surface object a Module3D will write to, and must match the dimensions
			// of the texture object the surface will read from.
			Module3D(int width, int height, int depth);

			// Destructor calls functions to clear CUDA objects/data
			virtual ~Module3D();

			// Conversion


			// Connects this Module3D to another source Module3D
			virtual void ConnectModule(Module3D* other);

			// Generates data and stores it in this object
			virtual void Generate() = 0;

			// Returns Generated data.
			virtual std::vector<float> GetData() const;

			std::vector<float> GetLayer(size_t idx) const;

			// Gets reference to module at given index in this modules "sourceModules" container
			virtual Module3D* GetModule(size_t idx) const;

			// Get number of source modules connected to this object.
			virtual size_t GetSourceModuleCount() const = 0;

			// Save noise at depth "idx" to output image.
			virtual void SaveToPNG(const char* name, size_t depth_idx);

			void SaveAllToPNGs(const char * base_name);

			// Tells us whether or not this module has already Generated data.
			bool Generated;

			// Each module will write self values into this, and allow other modules to read from this.
			// Allocated with managed memory.
			float* Output;

		protected:

			// Dimensions of textures.
			int3 Dimensions;

			// Modules that precede this module, with the back 
			// of the vector being the module immediately before 
			// this one, and the front of the vector being the initial
			// module.
			std::vector<Module3D*> sourceModules;
		};

        Module3D::Module3D(int width, int height, int depth) : Dimensions(make_int3(width, height, depth)) {
			Generated = false;
			// Allocate using managed memory, so that CPU/GPU can share a single pointer.
			// Be sure to call cudaDeviceSynchronize() before accessing Output.
			cudaError_t err = cudaSuccess;
			err = cudaMallocManaged(&Output, sizeof(float) * width * height * depth);
			cudaAssert(err);

			// Synchronize device to make sure we can access the Output pointer freely and safely.
			err = cudaDeviceSynchronize();
			cudaAssert(err);
		}

		Module3D::~Module3D(){
			cudaAssert(cudaDeviceSynchronize());
			cudaAssert(cudaFree(Output));
		}

		void Module3D::ConnectModule(Module3D * other){
			if (sourceModules.size() <= GetSourceModuleCount()) {
				sourceModules.push_back(other);
			}
			else {
				return;
			}
		}

		std::vector<float> Module3D::GetData() const{
			// Make sure to sync to device before accessing managed memory.
			cudaAssert(cudaDeviceSynchronize());
			return std::vector<float>(Output, Output + (Dimensions.x * Dimensions.y * Dimensions.z));
		}

		std::vector<float> Module3D::GetLayer(size_t idx) const {
			cudaAssert(cudaDeviceSynchronize());
			return std::vector<float>(Output + (idx * Dimensions.x * Dimensions.y), Output + (Dimensions.x * Dimensions.y));
		}

		Module3D* Module3D::GetModule(size_t idx) const{
			return sourceModules.at(idx);
		}

		void Module3D::SaveToPNG(const char * name, size_t depth_idx){
			if (depth_idx < 0 || depth_idx > Dimensions.z) {
				return;
			}
			std::vector<float> slice;
			slice = GetLayer(depth_idx);
			img::ImageWriter out(Dimensions.x, Dimensions.y);
			out.SetRawData(slice);
			out.WritePNG(name);
		}

		void Module3D::SaveAllToPNGs(const char* base_name) {

		}
