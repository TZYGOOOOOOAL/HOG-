#include "my_hog.h"

// Hog�����
cv::HOGDescriptor get_hog_desriptor(Size patch_size, Size block_size, Size block_stride,
	Size cell_size, int hist_bins)
{
	return HOGDescriptor(patch_size, block_size, block_stride, cell_size, hist_bins);
}

// ����ͼ��HOG����
vector<float> get_one_patch_hog_features(Mat &img, HOGDescriptor& hd, Size win_stride)
{
	vector<float> descriptors;					// hog������ ����
	hd.compute(img, descriptors, win_stride);	// ����hog�����ӣ���ⴰ���ƶ�������8��8��
	return descriptors;
}

// ת��HOG ������
vector<float> convert_svm_detector(const Ptr<ml::SVM>& svm)
{	
	Mat support_vectors = svm->getSupportVectors();
	
	Mat alpha, sv_idx;
	double rho = svm->getDecisionFunction(0, alpha, sv_idx);
	alpha.convertTo(alpha, CV_32FC1);

	Mat result_mat = alpha * support_vectors;
	result_mat *= -1.0f;
	
	vector<float> hog_detector(result_mat.cols + 1);

	for (int i = 0; i < result_mat.cols; i++)
		hog_detector[i] = result_mat.at<float>(0, i);

	hog_detector[support_vectors.cols] = (float)rho;

	return hog_detector;
}