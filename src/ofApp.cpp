#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    
    // load shaders
    depth.setGeometryInputType(GL_TRIANGLES);
    depth.setGeometryOutputType(GL_TRIANGLE_STRIP);
    depth.setGeometryOutputCount(18);
    depth.load("shaders/default.vert", "shaders/default.frag", "shaders/default.geom");
    depth.begin();
    depth.setUniform1f("farPlane", farPlane);
    depth.end();
    
    render.load("shaders/render");
    render.begin();
    render.setUniform1f("farPlane", farPlane);
    render.setUniform1i("depthMap", 1);
    render.end();
    
    // generate cubemap fbo
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (GLuint i = 0; i < 6; ++i){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, shadowMapRes, shadowMapRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // make projection matrix
    shadowProjMatrix.makePerspectiveMatrix(90.0, 1.0, 1.0, farPlane);
    
    // text
    setText();
        
    // set objects position
    textPos.set(-tbox.width/2, tbox.height/2, 0);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    time = ofGetElapsedTimef()/2;
    lightPos = ofVec3f(500 * sin(time), 250 + 200 * sin(2.0 * time), 500 * cos(time));
    
    // make view matricies
    lookAtMat[0].makeLookAtViewMatrix(lightPos, lightPos + ofVec3f(1,0,0), ofVec3f(0,-1,0));
    lookAtMat[1].makeLookAtViewMatrix(lightPos, lightPos + ofVec3f(-1,0,0), ofVec3f(0,-1,0));
    lookAtMat[2].makeLookAtViewMatrix(lightPos, lightPos + ofVec3f(0,1,0), ofVec3f(0,0,1));
    lookAtMat[3].makeLookAtViewMatrix(lightPos, lightPos + ofVec3f(0,-1,0), ofVec3f(0,0,-1));
    lookAtMat[4].makeLookAtViewMatrix(lightPos, lightPos + ofVec3f(0,0,1), ofVec3f(0,-1,0));
    lookAtMat[5].makeLookAtViewMatrix(lightPos, lightPos + ofVec3f(0,0,-1), ofVec3f(0,-1,0));
    
    // make view projection matricies
    for(int i=0;i<6;i++){
        viewProjMat[i] = lookAtMat[i] * shadowProjMatrix;
    }
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    time = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableDepthTest(); //深度テストを有効に
    ofBackground(255, 255, 255);
    
    // make depth cube map pass
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glViewport(0, 0, shadowMapRes, shadowMapRes);
    ofClear(0);
    depth.begin();
    depth.setUniform3f("lightPos", lightPos);
    glUniformMatrix4fv(glGetUniformLocation(depth.getProgram(), "viewProjMat"), 6, GL_FALSE, viewProjMat[0].getPtr());
    renderScene();
    depth.end();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // bind cubemap
    glActiveTexture( GL_TEXTURE0 + 1 );
    glEnable( GL_TEXTURE_CUBE_MAP );
    glBindTexture( GL_TEXTURE_CUBE_MAP, depthCubemap );
    
    // render pass
    cam.begin();
    render.begin();
    //ofVec4f ambientColor = ofVec4f(0.518, 0.631, 0.816, 1.0); //影色
    ofVec4f ambientColor = ofVec4f(0.1, 0.1, 0.1, 1.0); //影色
    render.setUniform3f("lightPos", lightPos);
    render.setUniform4f("ambientColor", ambientColor);
    renderScene();
    render.end();
    cam.end();
    
    // unbind cubemap
    glActiveTexture( GL_TEXTURE0 + 1 );
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0 );
    glDisable( GL_TEXTURE_CUBE_MAP );
    glActiveTexture( GL_TEXTURE0 );
}

//--------------------------------------------------------------
void ofApp::setText(){
    tdepth = 10; //テキストの奥行き
    glEnable(GL_DEPTH_TEST); //深度バッファを有効にする
    glDepthFunc(GL_LEQUAL); //深度バッファ
    font.loadFont("heisei.otf", 80, true, true, true); //フォント名、フォントサイズ
    typeStr = "hello world!";
    letterPaths = font.getStringAsPoints(typeStr);
    tbox = font.getStringBoundingBox(typeStr, 0, 0);
    
    for (int i = 0; i < letterPaths.size(); i++) {
        ofMesh front = letterPaths[i].getTessellation();
        ofMesh back = front;
        auto v = back.getVerticesPointer();
        for (int j = 0; j < (int)back.getNumVertices(); j++) {
            v[j].z += tdepth; //テキストの奥行き
            
            front.addNormal(ofVec3f(0, 0, -1));
            back.addNormal(ofVec3f(0, 0, 1));
        }
        textMesh.push_back(front);
        textMesh.push_back(back);
    }
    
    for (int i = 0; i < (int)letterPaths.size(); i++) {
        lines = letterPaths[i].getOutline();
        for (int j = 0; j < (int)lines.size(); j++) {
            if( j!= 0)ofNextContour(true) ;
            vector<glm::vec3> points = lines[j].getVertices();
            int k = 0;
            for (k = 0; k < (int)points.size()-1; k++) {
                ofPoint p1 = points[k+0];
                ofPoint p2 = points[k+1];
                
                side.addVertex(p1);
                side.addVertex(ofPoint(p1.x, p1.y, p1.z+tdepth));
                side.addVertex(p2);
                
                side.addVertex(ofPoint(p1.x, p1.y, p1.z+tdepth));
                side.addVertex(ofPoint(p2.x, p2.y, p2.z+tdepth));
                side.addVertex(p2);
            }
            // Connect the last to the first
            ofPoint p1 = points[k];
            ofPoint p2 = points[0];
            side.addVertex(p1);
            side.addVertex(ofPoint(p1.x, p1.y, p1.z+tdepth));
            side.addVertex(ofPoint(p2.x, p2.y, p2.z+tdepth));
            
            side.addVertex(p1);
            side.addVertex(ofPoint(p2.x, p2.y, p2.z+tdepth));
            side.addVertex(p2);
        }
    }
    side.setupIndicesAuto();
    setNormals(side);
    textMesh.push_back(side);
    
    /*
    for (int i = 0; i < textMesh.size(); i++) {
        for(int j = 0; j < textMesh[i].getVertices().size(); j++){
            //テキストの色
            textMesh[i].addColor(ofFloatColor(1.0, 0.0, 0.0, 1.0));
        }
    }
    */
}
//--------------------------------------------------------------
void ofApp::renderScene(){
    ofPushMatrix();
    ofRotateDeg(180, 1, 0, 0);
    ofTranslate(textPos);
    for (int i = 0; i < textMesh.size(); i++) {
        textMesh[i].draw();
    }
    ofPopMatrix();
    
    ofDrawBox(0, -tbox.height/2-5, 0, 10000, 1, 10000);
    
}

//--------------------------------------------------------------
void ofApp::setNormals(ofMesh &mesh){
    //The number of the vertices
    int nV = (int)mesh.getNumVertices();
    
    //The number of the triangles
    int nT = (int)mesh.getNumIndices()/3;
    
    vector<glm::vec3> norm(nV); //Array for the normals
    
    //Scan all the triangles. For each triangle add its
    //normal to norm's vectors of triangle's vertices
    for (int t = 0; t < nT; t++) {
        //Get indices of the triangle t
        int i1 = mesh.getIndex(3*t);
        int i2 = mesh.getIndex(3*t+1);
        int i3 = mesh.getIndex(3*t+2);
        
        //Get vertices of the triangle
        const glm::vec3 &v1 = mesh.getVertex(i1);
        const glm::vec3 &v2 = mesh.getVertex(i2);
        const glm::vec3 &v3 = mesh.getVertex(i3);
        
        //Compute the triangle's normal
        glm::vec3 dir = glm::normalize(glm::cross(v2-v1, v3-v1));
        
        //Accumulate it to norm array for i1, i2, i3
        norm[i1] += dir;
        norm[i2] += dir;
        norm[i3] += dir;
    }
    
    //Normalixe the normal's length
    for (int i = 0; i < nV; i++) {
        norm[i] = glm::normalize(norm[i]);
    }
    
    //Set the normals to mesh
    mesh.clearNormals();
    mesh.addNormals(norm);
    
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case 's': //take a screenshot
        case 'S':
            myImage.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
            myImage.save(ofGetTimestampString("%Y%m%d%H%M%S")+"##.png");
            break;
    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
