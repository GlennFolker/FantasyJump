#ifndef APP_LISTENER_H
#define APP_LISTENER_H

namespace Fantasy {
    class AppListener {
        public:
        AppListener(){}
        virtual ~AppListener(){}
        virtual void update() = 0;
    };
}

#endif
