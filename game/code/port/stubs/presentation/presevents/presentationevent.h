// Minimal PresentationEvent shim for the Linux PoC.
//
// BootupContext derives from PresentationEvent::PresentationEventCallBack and
// takes PresentationEvent* parameters.  The real PresentationEvent drags in the
// AnimationPlayer / Pure3D presentation stack, none of which the PoC bootup path
// exercises (the SHOW_MOVIES FMV queueing is compiled out).  Declare only the
// nested callback contract and the type name.
#ifndef PRESENTATIONEVENT_H
#define PRESENTATIONEVENT_H

class PresentationEvent
{
public:
    struct PresentationEventCallBack
    {
        virtual void OnPresentationEventBegin( PresentationEvent* pEvent ) = 0;
        virtual void OnPresentationEventLoadComplete( PresentationEvent* pEvent ) = 0;
        virtual void OnPresentationEventEnd( PresentationEvent* pEvent ) = 0;

    protected:
        virtual ~PresentationEventCallBack() = default;
    };
};

#endif // PRESENTATIONEVENT_H
