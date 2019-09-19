#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofVboMesh.h"
#include "MarkerRecognizer.h"
#include "AR_assistant.h"

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:
	void intrinsicMatrix2ProjectionMatrix(cv::Mat& camera_matrix, float width, float height, float near_plane, float far_plane, float* projection_matrix);
	void extrinsicMatrix2ModelViewMatrix(cv::Mat& rotation, cv::Mat& translation, float* model_view_matrix);

private:
	std::vector<ofxAssimpModelLoader> m_models;
	std::vector<ofMesh> m_meshes;
	ofLight	m_light;

	ofVideoGrabber m_video;
	//ofCamera m_camera;

	vector<cv::Point3f> m_corners_3d;

	cv::Mat m_camera_matrix;
	cv::Mat m_dist_coeff;
	float m_projection_matrix[16];
	float m_model_view_matrix[16];

	cv::Mat m_img_gray;
	cv::Mat m_img_color;
	MarkerRecognizer m_recognizer;
	AR_assistant assistant;
};
