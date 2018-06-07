#pragma once
// Automatically generated header file

#include "lwm2m_objects.hpp"
namespace KnownObjects {
namespace id3201 {
/* \brief Class for object 3201 - Digital Output
 *  Generic digital output for non-specific actuators
 */
class instance : public Lwm2mObjectInstance {
public:

    // 5550 - The current state of a digital output.
    bool DigitalOutputState;
    
    // Optional resource
    // 5551 - The polarity of a digital ouput as a Boolean (0 = Normal, 1= Reversed).
    bool DigitalOutputPolarity;
    
    // Optional resource
    // 5750 - The application type of the output as a string, for instance, “LED”
    PreallocString<30> ApplicationType;
    
    enum class RESID {
        DigitalOutputState = 5550,
        DigitalOutputPolarity = 5551,
        ApplicationType = 5750,
        
    };
};

/* \brief Class for object 3201 - Digital Output
 *  Generic digital output for non-specific actuators
 */
class object : public Lwm2mObject<3201, object, instance> {
public:

    // 5550 - The current state of a digital output.
    Resource(5550, &instance::DigitalOutputState, O_RES_RW) DigitalOutputState;
    
    // Optional resource
    // 5551 - The polarity of a digital ouput as a Boolean (0 = Normal, 1= Reversed).
    Resource(5551, &instance::DigitalOutputPolarity, O_RES_RW) DigitalOutputPolarity;
    
    // Optional resource
    // 5750 - The application type of the output as a string, for instance, “LED”
    Resource(5750, &instance::ApplicationType, O_RES_RW) ApplicationType;
    
};

} // end of id namespace
inline bool operator== (id3201::instance::RESID c1, uint16_t c2) { return (uint16_t) c1 == c2; }
inline bool operator== (uint16_t c2, id3201::instance::RESID c1) { return (uint16_t) c1 == c2; }

} // end of KnownObjects namespace
	