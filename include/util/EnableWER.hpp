#pragma once
#ifndef VULPES_ENABLE_WER_HPP
#define VULPES_ENABLE_WER_HPP
#ifdef _WIN32
namespace vulpes {
    namespace util {

        // try to edit registry values to enable memory dumping for this application.
        struct wer_enabler_t {
            
            wer_enabler_t() = default;

            void enable() const;

        };

    }
}
#endif
#endif //!VULPES_ENABLE_WER_HPP