= cribsim: cribbage simulator in C

agent guidelines:

* when fixing bugs, ALWAYS write a failing test first, then fix the bug
* run "make check" after every change
  * if the change is to add a failing test, you need to know it failed
  * if the change is anything else, the tests need to pass
* when code is correct, run "make format" to enforce consistent style
* when fixing bugs, prefer simple unit tests laser focused on the
  buggy code -- keep tests simple and don't over-engineer them
