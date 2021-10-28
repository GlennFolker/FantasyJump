#ifndef APP_LISTENER_H
#define APP_LISTENER_H

namespace Fantasy {
    class AppListener {
        public:
        AppListener(){}
        ~AppListener(){}

        virtual void update() = 0;
        virtual void dispose() {
            this->~AppListener();
        }
    };
}

#endif
