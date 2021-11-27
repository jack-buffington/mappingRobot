To see what is in the custom message type created here:

ros2 interface show jack_test/msg/SampleMessage

To publish a topic using this message:
ros2 topic pub --once /jj jack_test/msg/SampleMessage "{day: 12, month: 'theMonth', year: 1290}"


I found this youtube video to be helpful when working with custom messages:  https://www.youtube.com/watch?v=nG3IOkEOPps

It didn't cover how to publish to that topic from a C++ program though.
