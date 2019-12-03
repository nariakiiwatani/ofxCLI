# ofxCLI

command line interface for openFrameworks application.  
by typing some commands into the window like terminal you can call functions you set.

## usage
see example

```c++

---
ofApp.h
---
ofx::cli::Prompt prompt_;

---
ofApp.cpp
---
// lambda function (with or without capture)
prompt_.subscribe("basic", []() { cout << "basic event occured" << endl; });
// you can set function with arguments
prompt_.subscribe("int2", [](int _0, int _1) { cout << _0 << " " << _1 << endl; });
// you can specify default argument(s)
prompt_.subscribe("str", [](std::string s) { cout << s << endl; }, {"hoge"});
// many parameters...
prompt_.subscribe("many", std::function<void(int,int,int,int,int,int,int,int)>([](int _0, int _1, int _2, int _3, int _4, int _5, int _6, int _7) {
	cout << _0 << " " << _1 << " " << _2 << " " << _3 << " " << _4 << " " << _5 << " " << _6 << " " << _7 << endl;
}), {0,1,2,3,4,5,6,7});
// member function
prompt_.subscribe("some", this, &ofApp::someFunction);
// member function with default arguments
prompt_.subscribe("some2", this, &ofApp::someFunction, {100, 50.3f, "some string"});
```