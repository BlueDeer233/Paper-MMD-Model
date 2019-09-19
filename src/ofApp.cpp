#include "ofApp.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#define MULTIPLE 8.0f
const int frame_width = 640, frame_height = 480;

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofBackground(50, 0);
	ofSetWindowShape(frame_width, frame_height);

	ofDisableArbTex(); // we need GL_TEXTURE_2D for our models coords.

	m_video.setDeviceID(0);
	m_video.initGrabber(frame_width, frame_height);

	Point3f corners_3d[] =
	{
		Point3f(-MULTIPLE / 2, -MULTIPLE / 2, 0),
		Point3f(-MULTIPLE / 2,  MULTIPLE / 2, 0),
		Point3f(MULTIPLE / 2,  MULTIPLE / 2, 0),
		Point3f(MULTIPLE / 2, -MULTIPLE / 2, 0)
	};
	float camera_matrix[] =
	{
		1366.357063491539, 0, 977.839541212721,
		0, 1371.166623528788, 547.7999350167918,
		0, 0, 1
	};
	float dist_coeff[] = { 0.02654710903957197, 0.5578501525737973, -0.001865726692822726, 0.0004203059744919562, -2.121092807419418 };

	m_corners_3d = vector<Point3f>(corners_3d, corners_3d + 4);
	//Mat构造函数不会拷贝数据，只会将指针指向数据区域，所以对于局部变量内存，需要clone
	m_camera_matrix = Mat(3, 3, CV_32FC1, camera_matrix).clone();
	m_dist_coeff = Mat(1, 4, CV_32FC1, dist_coeff).clone();

	assistant.setup();
	assistant.road("resource/model/miku.pmx", "resource/action/hello.vmd", "resource/action/stand.vmd");
	assistant.update();
}


//--------------------------------------------------------------
void ofApp::update() {
	m_video.update();
	if (m_video.isFrameNew())
	{
		bool good_format = true;
		int width = m_video.getWidth();
		int height = m_video.getHeight();
		m_img_gray.create(height, width, CV_8UC1);

		ofPixelFormat format = m_video.getPixelFormat();
		switch (format)
		{
		case OF_PIXELS_RGB:
			m_img_color.create(height, width, CV_8UC3);
			memcpy(m_img_color.data, m_video.getPixels().getData(), height*width * 3);
			cvtColor(m_img_color, m_img_gray, CV_RGB2GRAY);
			break;
		case OF_PIXELS_RGBA:
			m_img_color.create(height, width, CV_8UC4);
			memcpy(m_img_color.data, m_video.getPixels().getData(), height*width * 4);
			cvtColor(m_img_color, m_img_gray, CV_RGBA2GRAY);
			break;
		case OF_PIXELS_BGRA:
			m_img_color.create(height, width, CV_8UC4);
			memcpy(m_img_color.data, m_video.getPixels().getData(), height*width * 4);
			cvtColor(m_img_color, m_img_gray, CV_BGRA2GRAY);
			break;
		default:
			good_format = false;
			cout << "Unsupported video format!" << endl;
			break;
		}
		if (!good_format) return;

		m_recognizer.update(m_img_gray, 150);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofSetColor(255);
	float view_width = ofGetViewportWidth();
	float view_height = ofGetViewportHeight();
	m_video.draw(0, 0, view_width, view_height);

	//Set camera matrix to the opengl projection matrix;
	intrinsicMatrix2ProjectionMatrix(m_camera_matrix, frame_width, frame_height, 0.01f*MULTIPLE, 100.0f*MULTIPLE, m_projection_matrix);
	//ofSetMatrixMode(OF_MATRIX_PROJECTION);
	//ofLoadIdentityMatrix();
	//Openframeworks里将(-1, -1)映射到屏幕左上角，而非一般的左下角，所以需要一个矩阵进行垂直镜像
	//static float reflect[] =
	//{
	//	1,  0, 0, 0,
	//	0, -1, 0, 0,
	//	0,  0, 1, 0,
	//	0,  0, 0, 1
	//};
	//ofLoadMatrix(reflect);
	//OpenGL默认为右乘
	//ofMultMatrix(m_projection_matrix);

	//Reset model view matrix to identity;
	//ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	//ofLoadIdentityMatrix();

	//ofDrawPlane(-1,-1,-1,2,2);

	//Set opengl parameters
	//ofSetColor(255);
	//ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofEnableDepthTest();
	//glShadeModel(GL_SMOOTH); //some model / light stuff
	//m_light.enable();
	//ofEnableSeparateSpecularLight();

	vector<Marker>& markers = m_recognizer.getMarkers();
	Mat r, t;
	static bool appear = false, helloed = false;
	static int interval = 5, fpsFrame = 0;
	static double fpsTime = saba::GetTime(), saveTime = saba::GetTime(), startTime = saba::GetTime();
	for (int i = 0; i < markers.size(); ++i)
	{
		//求出的旋转矩阵r的行列式为+1，即为刚体变换，所以不改变坐标系的手性
		markers[i].estimateTransformToCamera(m_corners_3d, m_camera_matrix, m_dist_coeff, r, t);
		extrinsicMatrix2ModelViewMatrix(r, t, m_model_view_matrix);
		//ofLoadMatrix(m_model_view_matrix);

		//ofSetColor(0x66, 0xcc, 0xff);
		//由于Marker坐标系与OpenCV坐标系的手性一致，所以Marker坐标系的Z轴垂直于Marker向下
		//绘制Box时的Anchor在Box中心，所以需要-0.5*size的偏移才能使Box的底面在Marker上！！
		//float size = (float)markers[i].m_id / 1024;
		//ofDrawBox(0, 0, -0.5*size*MULTIPLE, 0.8f*MULTIPLE, 0.8f*MULTIPLE, size*MULTIPLE);
		//ofDrawBox(0, 0, -0.4f*MULTIPLE, 0.8f*MULTIPLE);
		//ofRotateXDeg(90);
		//ofScale(0.05, 0.05, 0.05);
		double time = saba::GetTime();
		double elapsed = time - saveTime;
		if (elapsed > 1.0 / 30.0)
		{
			elapsed = 1.0 / 30.0;
		}
		saveTime = time;
		assistant.m_appContext.m_elapsed = float(elapsed);
		assistant.m_appContext.m_animTime += float(elapsed);

		assistant.m_appContext.m_screenWidth = view_width;
		assistant.m_appContext.m_screenHeight = view_height;

		assistant.update();
		assistant.draw();

		if (time - startTime >= interval)
		{
			startTime = time;
			assistant.m_appContext.m_elapsed = 0.0f;
			assistant.m_appContext.m_animTime = 0.0f;
			if (!helloed)
			{
				assistant.m_model.m_vmdAnim = assistant.m_vmdAnims[1];
				interval = 3;
			}
			helloed = true;
		}

		//cout << i << ':';
		//for (int j = 0; j < 4; j++) {
		//	cout << markers[i].m_corners[j];
		//	if (j == 3) cout << ':';
		//	else cout << ',';
		//}
		//cout << markers[i].m_id << endl;
	}
	if (markers.size())
	{
		//cout << endl;
		appear = true;
	}
	else
	{
		saveTime = startTime = saba::GetTime();
		if (appear)
		{
			assistant.m_appContext.m_elapsed = 0.0f;
			assistant.m_appContext.m_animTime = 0.0f;
			assistant.m_model.m_vmdAnim = assistant.m_vmdAnims[0];

			interval = 5; helloed = false;
			appear = false;
		}
	}

	fpsFrame++;
	double time = saba::GetTime();
	double deltaTime = time - fpsTime;
	if (deltaTime > 1.0)
	{
		double fps = double(fpsFrame) / deltaTime;
		std::cout << fps << " fps " << startTime << " " << saveTime << std::endl;
		fpsFrame = 0;
		fpsTime = time;
	}

	//Reset parameters
	ofDisableDepthTest();
	//m_light.disable();
	//ofDisableLighting();
	//ofDisableSeparateSpecularLight();

	//ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	//ofLoadIdentityMatrix();
	//ofSetMatrixMode(OF_MATRIX_PROJECTION);
	//ofLoadIdentityMatrix();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 's')
	{
		string name = ofGetTimestampString();
		imwrite(name + ".jpg", m_img_gray);
		cout << "Frame " << name << " has been saved!" << endl;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

void ofApp::intrinsicMatrix2ProjectionMatrix(cv::Mat& camera_matrix, float width, float height, float near_plane, float far_plane, float* projection_matrix)
{
	float f_x = camera_matrix.at<float>(0, 0);
	float f_y = camera_matrix.at<float>(1, 1);

	float c_x = camera_matrix.at<float>(0, 2);
	float c_y = camera_matrix.at<float>(1, 2);

	projection_matrix[0] = 2 * f_x / width;
	projection_matrix[1] = 0.0f;
	projection_matrix[2] = 0.0f;
	projection_matrix[3] = 0.0f;

	projection_matrix[4] = 0.0f;
	projection_matrix[5] = 2 * f_y / height;
	projection_matrix[6] = 0.0f;
	projection_matrix[7] = 0.0f;

	projection_matrix[8] = 1.0f - 2 * c_x / width;
	projection_matrix[9] = 2 * c_y / height - 1.0f;
	projection_matrix[10] = -(far_plane + near_plane) / (far_plane - near_plane);
	projection_matrix[11] = -1.0f;

	projection_matrix[12] = 0.0f;
	projection_matrix[13] = 0.0f;
	projection_matrix[14] = -2.0f*far_plane*near_plane / (far_plane - near_plane);
	projection_matrix[15] = 0.0f;

	glm::mat4 assistant_project = glm::mat4(m_projection_matrix[0], m_projection_matrix[1], m_projection_matrix[2], m_projection_matrix[3],
		m_projection_matrix[4], m_projection_matrix[5], m_projection_matrix[6], m_projection_matrix[7],
		m_projection_matrix[8], m_projection_matrix[9], m_projection_matrix[10], m_projection_matrix[11],
		m_projection_matrix[12], m_projection_matrix[13], m_projection_matrix[14], m_projection_matrix[15]);
	//glm::mat4 rotateXY = glm::mat4(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	//assistant_project = assistant_project * rotateXY;
	assistant.m_appContext.m_projMat = assistant_project;
}

void ofApp::extrinsicMatrix2ModelViewMatrix(cv::Mat& rotation, cv::Mat& translation, float* model_view_matrix)
{
	//绕X轴旋转180度，从OpenCV坐标系变换为OpenGL坐标系
	static double d[] =
	{
		1, 0, 0,
		0, -1, 0,
		0, 0, -1
	};
	Mat_<double> rx(3, 3, d);

	rotation = rx * rotation;
	translation = rx * translation;

	model_view_matrix[0] = rotation.at<double>(0, 0);
	model_view_matrix[1] = rotation.at<double>(1, 0);
	model_view_matrix[2] = rotation.at<double>(2, 0);
	model_view_matrix[3] = 0.0f;

	model_view_matrix[4] = rotation.at<double>(0, 1);
	model_view_matrix[5] = rotation.at<double>(1, 1);
	model_view_matrix[6] = rotation.at<double>(2, 1);
	model_view_matrix[7] = 0.0f;

	model_view_matrix[8] = rotation.at<double>(0, 2);
	model_view_matrix[9] = rotation.at<double>(1, 2);
	model_view_matrix[10] = rotation.at<double>(2, 2);
	model_view_matrix[11] = 0.0f;

	model_view_matrix[12] = translation.at<double>(0, 0);
	model_view_matrix[13] = translation.at<double>(1, 0);
	model_view_matrix[14] = translation.at<double>(2, 0);
	model_view_matrix[15] = 1.0f;

	glm::mat4 assistant_modelview = glm::mat4(m_model_view_matrix[0], m_model_view_matrix[1], m_model_view_matrix[2], m_model_view_matrix[3],
		m_model_view_matrix[4], m_model_view_matrix[5], m_model_view_matrix[6], m_model_view_matrix[7],
		m_model_view_matrix[8], m_model_view_matrix[9], m_model_view_matrix[10], m_model_view_matrix[11],
		m_model_view_matrix[12], m_model_view_matrix[13], m_model_view_matrix[14], m_model_view_matrix[15]);
	glm::mat4 rotateYZ = glm::mat4(1, 0, 0, 0,
								   0, 0, -1, 0,
								   0, 1, 0, 0,
								   0, 0, 0, 1);
	assistant.m_appContext.m_viewMat = assistant_modelview * rotateYZ;
}
