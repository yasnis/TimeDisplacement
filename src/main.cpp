#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{
    enum AppMode {
        AppMode_Release,
        AppMode_Debug
    };
    
    AppMode mode = AppMode_Debug;
    ofxPanel gui;
	//ofParameter<string> param_framerate;
	ofParameter<bool> playFlag = true;
	ofParameterGroup gui_type;
	ofxToggle mode_noise;
	ofxToggle mode_vertical;
	ofxToggle mode_horizontal;

	ofParameterGroup gui_noise;
	ofParameter<float> param_noise_x_scale;
	ofParameter<float> param_noise_y_scale;
	ofParameter<float> param_noise_x_offset;
	ofParameter<float> param_noise_y_offset;
	ofParameter<float> param_noise_time_offset;
	ofParameter<float> param_noise_time_scale;
    
	ofVideoGrabber video;
	vector<ofFbo *> caches;
	ofImage mapImage;
	ofPixels map;
	ofFbo dest_image;

	ofShader shader;

	ofFbo texture_tile;

	int video_num = 49;

	int camWidth = 640;
	int camHeight = 480;

	int noiseWidth = 240;
	int noiseHeight = 180;
    
    //--------------------------------------------------------------
    void setup(){
        ofBackground(0);
        ofSetFrameRate(60);
        ofSetVerticalSync(true);
        
        setupGUI();

		//video.setup(640, 360);
		video.setDeviceID(0);
		video.setDesiredFrameRate(60);
		video.initGrabber(camWidth, camHeight);

		for (int i = 0; i < video_num; i++){
			ofFbo *fbo = new ofFbo();
			fbo->allocate(camWidth, camHeight);
			fbo->begin();
			ofSetColor(0);
			ofDrawRectangle(0, 0, camWidth, camHeight);
			fbo->end();
			caches.push_back(fbo);
		}
		map.allocate(noiseWidth, noiseHeight, OF_IMAGE_GRAYSCALE);
		dest_image.allocate(camWidth, camHeight);

		shader.load("displacement.vert", "displacement.frag");

		texture_tile.allocate(camWidth * 7, camHeight * 7);
    }
    
    void setupGUI() {
        gui.setDefaultWidth(240);
        gui.setup();
        gui.setName("Setting");
		gui.add(playFlag.set("Play", true));
		gui_type.setName("Type");
		mode_noise.setup("Noise", false);
		mode_vertical.setup("Vertical", false);
		mode_horizontal.setup("Horizontal", false);
		mode_noise.addListener(this, &ofApp::setModeNoise);
		mode_vertical.addListener(this, &ofApp::setModeVertical);
		mode_horizontal.addListener(this, &ofApp::setModeHorizontal);

		gui_type.add(mode_noise.getParameter());
		gui_type.add(mode_vertical.getParameter());
		gui_type.add(mode_horizontal.getParameter());

		gui_noise.setName("Perlin Noise");
		gui_noise.add(param_noise_x_scale.set("X Scale", 0.01, 0.0, 0.05));
		gui_noise.add(param_noise_y_scale.set("Y Scale", 0.01, 0.0, 0.05));
		gui_noise.add(param_noise_x_offset.set("X Offset", 0.3, 0.0, noiseWidth));
		gui_noise.add(param_noise_y_offset.set("Y Offset", 0.3, 0, noiseHeight));
		gui_noise.add(param_noise_time_scale.set("Time Scale", 0.3, 0.0, 1.0));
		gui_noise.add(param_noise_time_offset.set("Time Offset", 0.0, 0, noiseWidth));

		gui.add(gui_type);
		gui.add(gui_noise);
        
        gui.loadFromFile("settings.xml");
    }
	//--------------------------------------------------------------
	void ofApp::setModeNoise(bool &value) {
		if (!value) return;
		mode_vertical = false;
		mode_horizontal = false;
	}
	void ofApp::setModeVertical(bool &value) {
		if (!value) return;
		mode_noise = false;
		mode_horizontal = false;
	}
	void ofApp::setModeHorizontal(bool &value) {
		if (!value) return;
		mode_vertical = false;
		mode_noise = false;
	}
    
    //--------------------------------------------------------------
    void update(){
		video.update();
		updateMap();
		ofSetWindowTitle(ofToString(ofGetFrameRate())+" fps");
    }
	void updateMap() {
		if (!playFlag) return;
		float time = ofGetElapsedTimef();
		if (mode_noise) {
			float x_scale = param_noise_x_scale;		//1.0 / scaleX is coherence in x
			float y_scale = param_noise_y_scale;		//1.0 / scaleY is coherence in y
			float x_offset = param_noise_x_offset;
			float y_offset = param_noise_y_offset;
			float time_scale = param_noise_time_scale;
			float time_offset = param_noise_time_offset;
			for (int y = 0; y<map.getHeight(); y++) {
				for (int x = 0; x<map.getWidth(); x++) {
					float value = ofNoise(x*x_scale + x_offset, y*y_scale + y_offset, time*time_scale + time_offset);
					map.setColor(x, y, ofColor(value * 255));
				}
			}
		} else if (mode_vertical) {
			float timeDelta = fmod(time*10, map.getWidth());
			for (int y = 0; y<map.getHeight(); y++) {
				for (int x = 0; x<map.getWidth(); x++) {
					float value = 0.9 - fmod(x, map.getWidth())/ map.getWidth()*0.8;
					map.setColor(x, y, ofColor(value * 255));
				}
			}
		} else {
			float timeDelta = fmod(time * 10, map.getWidth());
			for (int y = 0; y<map.getHeight(); y++) {
				for (int x = 0; x<map.getWidth(); x++) {
					float value = 0.9 - fmod(y, map.getHeight()) / map.getHeight()*0.8;
					map.setColor(x, y, ofColor(value * 255));
				}
			}
		}
		mapImage.setFromPixels(map);
	}
    //--------------------------------------------------------------
    void draw() {
		updateFBO();
		video.draw(0, 0);
		
		drawFBO();
		texture_tile.draw(0, camHeight, camWidth, camHeight);

		drawDestImage();

		dest_image.draw(camWidth, camHeight, camWidth, camHeight);
		mapImage.draw(camWidth, 0, camWidth, camHeight);
        if(mode == AppMode_Debug) {
            drawDebug();
        }
    }
	void drawDestImage() {

		dest_image.begin();
		shader.begin();

		shader.setUniformTexture("map", mapImage.getTexture(), 100);
		shader.setUniform2f("resolution", camWidth, camHeight);
		shader.setUniform1f("time", ofGetElapsedTimef());
		shader.setUniform1f("scale", ((float)noiseWidth)/camWidth);
		for (int i = 0; i < video_num; i++) {
			shader.setUniformTexture("texture"+ofToString(i), *caches.at(i), i);
		}
		shader.setUniformTexture("texture", texture_tile, 101);

		ofDrawRectangle(0, 0, dest_image.getWidth(), dest_image.getHeight());
		shader.end();

		dest_image.end();
	}
	void updateFBO() {
		if (caches.size() > 0) {
			ofFbo * fbo = caches.back();
			fbo->begin();
			video.draw(0, 0, fbo->getWidth(), fbo->getHeight());
			fbo->end();
			caches.insert(caches.begin(), fbo);
			caches.pop_back();
		}
	}
	void drawFBO() {
		int x = 0;
		int y = 0;

		texture_tile.begin();
		auto it = caches.begin();
		while (it != caches.end()) {
			(*it)->draw(x, y, (*it)->getWidth(), (*it)->getHeight());
			/*
			ofSetColor((float)x/ texture_tile.getWidth()*255, (float)y/ texture_tile.getHeight() * 255, 0);
			ofDrawRectangle(x, y, camWidth/2, 50);
			ofSetColor(0, 0, (float)(y/camHeight*7+x/camWidth)/video_num*255);
			ofDrawRectangle(x+ camWidth / 2, y, camWidth / 2, 50);
			ofSetColor(255);
			//*/
			x += (*it)->getWidth();
			if (x >= texture_tile.getWidth()) {
				x = 0;
				y += (*it)->getHeight();
			}
			it++;
		}
		texture_tile.end();
	}

    
    void drawDebug() {
        ofSetColor(255);
        gui.draw();
    }
    
    void exit() {
        gui.saveToFile("settings.xml");
    }
    
    void switchDebug() {
        if (mode == AppMode_Debug) {
            mode = AppMode_Release;
        }else{
            mode = AppMode_Debug;
        }
    }
    
    void keyPressed(int key){
        if(key == 'f'){
            ofToggleFullscreen();
        }else if(key == 'd') {
            switchDebug();
        }
    }
    
    void updateFloatParam(float &value) {
        //do something.
        cout << "updateFloatParam : " << value << endl;
    }
    
    void updateIntParam(int &value) {
        //do something.
        cout << "updateIntParam : " << value << endl;
    }
};

//========================================================================
int main( ){
	ofSetupOpenGL(1280,960, OF_WINDOW);
	ofRunApp( new ofApp());
}
