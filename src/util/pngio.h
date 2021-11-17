#ifndef PNGIO_H
#define PNGIO_H

#include <string>

namespace Fantasy {
    struct Png {
        public:
        bool errored;
        int width;
        int height;
        std::string filename;
        unsigned char **data;

        public:
        Png(const std::string &);
        Png(int, int);
        ~Png();

        void write(const std::string &) const;
    };
}

#endif
