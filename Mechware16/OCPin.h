#include <DigitalIO.h>

// This class is inspired to DigitalIO's DigitalPin class
template<uint8_t pinNumber>
class OpenCollectorPin {
public:
        OpenCollectorPin () {
                // Start as INPUT, i.e. Hi-Z, implicitly HIGH
                fastPinMode (pinNumber, INPUT);
                fastDigitalWrite (pinNumber, false);    // Make sure pull-up is disabled
        }

        inline operator bool () const __attribute__((always_inline)) {
                return fastDigitalRead (pinNumber);
        }

        inline __attribute__((always_inline))
        void high () {
                // Stop driving the line and let the pull-up resistor do the job
                fastPinMode (pinNumber, INPUT);
        }

        inline __attribute__((always_inline))
        void low () {
                // Bring down the line!
                fastPinMode (pinNumber, OUTPUT);                // Implicitly low
        }
};
