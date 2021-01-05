# ifndef __CONFIG_H__
# define __CONFIG_H__

#include <opencv.hpp>
#include <string>
#include <vector>

using namespace std;
using namespace cv;

struct Config
{
	// �������ݼ�
	string TRAIN_POS_RAW_IMG_DIR = "data/normalized_images/train/pos";
	string TRAIN_NEG_RAW_IMG_DIR = "data/original_images/train/neg";

	string VALID_POS_RAW_IMG_DIR = "data/normalized_images/test/pos";
	string VALID_NEG_RAW_IMG_DIR = "data/original_images/test/neg";

	string TRAIN_POS_IMG_DIR = "data/train/pos";
	string TRAIN_NEG_IMG_DIR = "data/train/neg";
	string VALID_POS_IMG_DIR = "data/valid/pos";
	string VALID_NEG_IMG_DIR = "data/valid/neg";

	string TRAIN_HARD_IMG_DIR = "data/train/hard";

	Size PATCH_SIZE = Size(64, 128);
	float POS_NEG_RATIO = 3.0f;

	// Hog ��������
	Size HOG_BLOCK_SIZE = Size(16, 16);
	Size HOG_BLOCK_STRIDE = Size(8, 8);
	Size HOG_CELL_SIZE = Size(8, 8);
	int  HOG_BINS = 9;

	// ��������
	string TRAIN_FEATURE_PATH = "data/features/train_features.xml";
	string VALID_FEATURE_PATH = "data/features/valid_features.xml";
	string TRAIN_LABEL_PATH = "data/features/train_labels.xml";
	string VALID_LABEL_PATH = "data/features/valid_labels.xml";

	string TRAIN_HARD_FEATURE_PATH = "data/features/train_hard_features.xml";
	string TRAIN_HARD_LABEL_PATH = "data/features/train_hard_labels.xml";


	// SVMѵ������
	ml::SVM::KernelTypes SVM_KERNEL_TYPE = ml::SVM::KernelTypes::LINEAR;
	Mat CLASS_WEIGHT = Mat(vector<float>({ 0.3f, 0.7f }));
	unsigned int SVM_TRAIN_ITER = 10000;
	bool TRAIN_AUTO = true;
	int TRAIN_K_FLOD = 5;
	string SVM_MODEL_SAVE_DIR  = "models/svm";
	string SVM_MODEL_PATH = "models/svm/save/m_hard_auto.xml";
	string SVM_HARD_MODEL_PATH = "models/svm/m_hard_auto.xml";

	string HOG_DETECT_MODEL_PATH = "models/hog/hog_0.xml";

	/*** ��� ***/
	// ����·��
	string TEST_IMG_DIR = "data/test/img";
	string TEST_TXT_DIR = "data/test/txt";

	// ���ģ�� �Լ� ���÷���
	bool TEST_WITH_LABEL = true;
	bool USE_CV_METHOD = true;
	bool USE_DEFAULT_MODEL = false;

	// ������
	Size WIN_STRIDE = Size(8, 8);		// ���ڻ�������, ��ȡ������
	Size DETECT_STEP = Size(8, 8);		// ���ڻ�������������� 8�� 8
	Size DETECT_PAD = Size(32, 32);
	double DETECT_SCALE = 1.1;			// 1.05
	double DETECT_CONF_TH = 0.7;		// 0.7
	double DETECT_NMS_TH = 0.3;
	double DETECT_IOU_TH = 0.3;
	
	// ���ӻ�����
	bool VISUAL_TEST_RESULT = true;
	Scalar VISUAL_PREDICT_COLOR = Scalar(0, 0, 255);
	Scalar VISUAL_LABEL_COLOR = Scalar(255, 0, 0);
	int VISUAL_THICKNESS = 1;

	// �������
	bool SAVE_RESULT = false;
	string RESULT_SAVE_DIR = "result/no-hard";

};

# endif