#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    ofGLWindowSettings settings;
    settings.setGLVersion(4, 0);
    settings.setSize(1024, 768);
    ofCreateWindow(settings);
    ofRunApp(new ofApp());
}
