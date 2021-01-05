#include "test.h"
#include "Config.h"
#include "utils.h"
#include "my_hog.h"
#include "eval.h"

extern Config config;

void test()
{
	Timer timer;

	/*** ��ȡ���� ***/
	// ͼ������
	vector<string> img_paths = get_child_files(config.TEST_IMG_DIR);
	//vector<string> img_paths({ "data/test/crop_000002.png" });

	// ��ǩ
	vector<vector<Rect>> labels;
	if (config.TEST_WITH_LABEL)
		labels = parse_annotations(img_paths, config.TEST_TXT_DIR);

	// hog �����
	HOGDescriptor hd = get_hog_desriptor(config.PATCH_SIZE, config.HOG_BLOCK_SIZE, config.HOG_BLOCK_STRIDE,
		config.HOG_CELL_SIZE, config.HOG_BINS);

	// ���н��
	vector<vector<Rect>> predicts;

	/************************* �Լ�ʵ�ּ�ⷽ�� ******************************/
	if (!config.USE_CV_METHOD)
	{
		// ����SVMģ��
		if (!is_file(config.SVM_MODEL_PATH))
		{
			cerr << "SVM model NOT Exits !!!";
			return;
		}
		Ptr<ml::SVM> svm = load_svm(config.SVM_MODEL_PATH);

		// ����ÿ��ͼ��
		timer.reset();
		for (int i = 0; i < img_paths.size(); i++)
		{
			//if (get_basename(img_paths[i]) != "crop_000002.png")
			//	continue;
			vector<Rect> bboxes;
			vector<double> confs;
			Mat img = imread(img_paths[i]);

			cout << "Test " << img_paths[i] << endl;
			test_one_image_on_my_method(img, hd, svm, bboxes, confs, config.DETECT_STEP,
				config.DETECT_PAD, config.DETECT_SCALE, config.DETECT_CONF_TH, config.DETECT_NMS_TH);
			predicts.push_back(bboxes);

			// ���ӻ�
			if (config.VISUAL_TEST_RESULT || config.SAVE_RESULT)
			{
				if (config.TEST_WITH_LABEL)
					img = visual_bboxes(img, labels[i], vector<double>(), config.VISUAL_LABEL_COLOR, config.VISUAL_THICKNESS, false);
				img = visual_bboxes(img, bboxes, confs, config.VISUAL_PREDICT_COLOR, config.VISUAL_THICKNESS, config.VISUAL_TEST_RESULT);

				if (config.SAVE_RESULT)
				{
					make_dir(config.RESULT_SAVE_DIR);
					string result_save_path = path_join(config.RESULT_SAVE_DIR, get_filename(img_paths[i]) + ".jpg");
					cv::imwrite(result_save_path, img);
				}
			}
		}

		double total_time = timer.get_run_time("", false, false);
		cout << "my method test time per img : " << total_time / img_paths.size() << "(s)" << endl;
	}

	/*************************** OpenCV�Դ����� ********************************/
	else
	{
		cout << "Loading Test Model ..." << endl;
		// ���ؼ��ģ��
		if (config.USE_DEFAULT_MODEL){
			hd.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
		}
		else
		{
			// ������ת���õģ�������make�µ�
			//if (is_file(config.HOG_DETECT_MODEL_PATH))
				//hd.load(config.HOG_DETECT_MODEL_PATH);
			//else
			{
				if (is_file(config.SVM_MODEL_PATH))
				{
					Ptr<ml::SVM> svm = load_svm(config.SVM_MODEL_PATH);

					// OpenCVֻ֧�����Ժ�
					if (svm->getKernelType() != ml::SVM::KernelTypes::LINEAR)
					{
						cerr << "SVM NOT Linear Kernel  !!!";
						return;
					}
					vector<float> hog_detector = convert_svm_detector(svm);
					hd.setSVMDetector(hog_detector);

					// ����ת�����
					hd.save(config.HOG_DETECT_MODEL_PATH);
				}

				// ģ��·��������
				else
				{
					cerr << "SVM model :" << config.SVM_MODEL_PATH << " Not Exist !!!";
					return;
				}

			}
		}

		// ����ÿ��ͼ��
		timer.reset();
		cout << "Start Testing !!!" << endl;
		for (int i = 0; i < img_paths.size(); i++)
		//labels.erase(labels.begin() + 10, labels.end());
		//for (int i = 0; i < 10; i++)
		{
			vector<Rect> bboxes;
			vector<double> confs;
			Mat img = imread(img_paths[i]);

			cout << "Test " << img_paths[i] << endl;
			test_one_image_on_cv_method(img, hd, bboxes, confs, config.DETECT_STEP,
				config.DETECT_PAD, config.DETECT_SCALE, config.DETECT_CONF_TH, config.DETECT_NMS_TH);
			predicts.push_back(bboxes);
				
			// ���ӻ�
			if (config.VISUAL_TEST_RESULT || config.SAVE_RESULT)
			{
				if (config.TEST_WITH_LABEL)
					img = visual_bboxes(img, labels[i], vector<double>(), config.VISUAL_LABEL_COLOR, config.VISUAL_THICKNESS, false);
				img = visual_bboxes(img, bboxes, confs, config.VISUAL_PREDICT_COLOR, config.VISUAL_THICKNESS, config.VISUAL_TEST_RESULT);

				if (config.SAVE_RESULT)
				{
					make_dir(config.RESULT_SAVE_DIR);
					string result_save_path = path_join(config.RESULT_SAVE_DIR, get_filename(img_paths[i]) + ".jpg");
					cv::imwrite(result_save_path, img);
				}
			}
				
		}
		double total_time = timer.get_run_time("", false, false);
		cout << "opencv detect test time per img : " << total_time / img_paths.size() << "(s)" << endl;
	}

	/*** ���۽�� ***/
	if (config.TEST_WITH_LABEL)
	{
		eval_detect(predicts, labels, config.DETECT_IOU_TH);
	}
}


// �Լ�ʵ�ֵķ���
void test_one_image_on_my_method(Mat &img, HOGDescriptor &hd, Ptr<ml::SVM> &svm, vector<Rect> &bboxes, vector<double> &confs,
	const Size &step_size, const Size &pad_size, const double &det_scale, const double &conf_th, const double &nms_th)
{
	// ���߶�
	double max_scale = pow(det_scale, 10);

	// ���20���߶�
	double s = max_scale;
	for (int i = 0; i < 20; i++)
	{ 
		s /= det_scale;

		if (int(img.rows*s) < hd.winSize.height || int(img.cols*s) < hd.winSize.width)
			break;

		Mat scale_img;
		resize(img, scale_img, Size(0, 0), s, s);

		vector<Rect> one_img_bboxes;
		vector<double> one_img_confs;

		test_one_image_on_one_scale(scale_img, hd, svm, one_img_bboxes, one_img_confs, step_size, pad_size);

		// bboxes ��ԭ��ԭ�߶���
		for (int bbox_idx = 0; bbox_idx < one_img_bboxes.size(); bbox_idx++)
		{
			one_img_bboxes[bbox_idx].x = int(one_img_bboxes[bbox_idx].x / s) - pad_size.width;
			one_img_bboxes[bbox_idx].y = int(one_img_bboxes[bbox_idx].y / s) - pad_size.height;
			one_img_bboxes[bbox_idx].width = int(one_img_bboxes[bbox_idx].width / s);
			one_img_bboxes[bbox_idx].height = int(one_img_bboxes[bbox_idx].height / s);
		}

		bboxes.insert(bboxes.end(), one_img_bboxes.begin(), one_img_bboxes.end());
		confs.insert(confs.end(), one_img_confs.begin(), one_img_confs.end());
	}

	// ����
	nms(bboxes, confs, conf_th, nms_th);

	return;
}

// ���߶�Ԥ��
void test_one_image_on_one_scale(Mat &img, HOGDescriptor &hd, Ptr<ml::SVM> &svm,
	vector<Rect> &bboxes, vector<double> &confs,const Size &step_size,const Size &pad_size)
{
	int pad_v = pad_size.height;
	int pad_h = pad_size.width;
	cv::copyMakeBorder(img, img, pad_v, pad_v, pad_h, pad_h, cv::BORDER_REFLECT101);

	int win_h = hd.winSize.height;
	int win_w = hd.winSize.width;
	int step_h = step_size.height;
	int step_w = step_size.width;
	Mat feature_mat, predict_mat;

	vector<vector<float>> features;
	vector<Rect> ROI_bboxes;

	for (int i = 0; i < img.rows - win_h; i += step_h)
	{
		for (int j = 0; j < img.cols - win_w; j += step_w)
		{
			Rect ROI_rect(j, i, win_w, win_h);
			Mat ROI = img(ROI_rect).clone();
			ROI_bboxes.push_back(ROI_rect);
			features.emplace_back(get_one_patch_hog_features(ROI, hd, Size(8,8)));
		}
	}
	
	// �������ݾ���
	feature_mat = Mat(features.size(), features[0].size(), CV_32FC1);
	for (int i = 0; i < features.size(); i++)
	{
		for (int j = 0; j < features[i].size(); j++)
			feature_mat.at<float>(i, j) = features[i][j];
	}

	// Ԥ�� ���ԭʼֵ������������룩
	svm->predict(feature_mat, predict_mat, ml::StatModel::Flags::RAW_OUTPUT);

	// �����д���0����������������ת�����Ŷȣ�������
	for (int i = 0; i < predict_mat.rows; i++)
	{
		double dist = (double)(predict_mat.at<float>(i, 0));
		if (dist < 0)  // �п� < 0 ����������
		{
			confs.push_back(cvt_to_conf(-dist));
			bboxes.push_back(ROI_bboxes[i]);
			//Mat ROI = img(ROI_bboxes[i]).clone();
			//imshow("", ROI);
			//waitKey();
		}
	}
	return;
}


void test_one_image_on_cv_method(Mat &img, HOGDescriptor &hd, vector<Rect> &bboxes, vector<double> &confs, 
	const Size &step_size, const Size &pad_size, const double &det_scale, const double &conf_th, const double &nms_th)
{
	// scale ��ʾ ͼ�񲻶���С 1/scale ֱ��ͼ���С <= ���ڴ�С������64�ε�����
	// 3: ���߽���룬 4��������5��pad��6��scale
	hd.detectMultiScale(img, bboxes, confs, 0.0, step_size, pad_size, det_scale);

	// ת�����Ŷ�
	for (int i = 0; i < bboxes.size(); i++)
		confs[i] = cvt_to_conf(confs[i]);

	// nms ����
	nms(bboxes, confs, conf_th, nms_th);
	return;
}
//
//int main()
//{
//	Mat image = imread("test.jpg");
//	// 1. ����HOG����
//	HOGDescriptor hog(Size(64, 128), Size(16, 16), Size(8, 8), Size(8, 8), 9);//HOG���������������HOG�����ӵ�
//	// 2. ����SVM������
//	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());   // �����Ѿ�ѵ���õ����˼�������
//	// 3. �ڲ���ͼ���ϼ����������
//	vector<cv::Rect> regions;
//	hog.detectMultiScale(image, regions, 0, Size(8, 8), Size(32, 32), 1.05, 1);
//	// ��ʾ
//	for (size_t i = 0; i < regions.size(); i++)
//	{
//		rectangle(image, regions[i], Scalar(0, 0, 255), 2);
//	}
//	imshow("HOG���˼��", image);
//	waitKey();
//
//	return 0;


//float distanceSample(cv::Mat &sample)
//{
//	assert(svm != NULL && svm->isTrained());
//	assert(!sample.empty());
//
//	cv::Mat result;
//	svm->predict(sample, result, cv::ml::StatModel::Flags::RAW_OUTPUT);
//	float dist = result.at<float>(0, 0);
//	return dist;
//}
//...
//
//float dist = distanceSample(yourSample);
//float confidence = (1.0 / (1.0 + exp(-dist)));

void nms(vector<Rect> &bboxes, vector<double> &confs, const double &conf_th, const double &nms_th)
{
	int SIZE = bboxes.size();
	if (SIZE <= 0)
		return;
	vector<bool> keep_idxs(SIZE, true);
	
	// Step1: ��������
	vector<int> idxs = argsort_d(confs);
	vector<double> confs_old = confs;
	vector<Rect> bboxes_old = bboxes;
	for (int i = 0; i < SIZE; i++)
	{
		confs[i] = confs_old[idxs[i]];
		bboxes[i] = bboxes_old[idxs[i]];
	}

	// Step2: ȥ�����ŶȽϵ͵�
	for (int i = 0; i < SIZE; i++)
	{
		if (confs[i] < conf_th)
			keep_idxs[i] = false;
	}

	// Step3: nms
	for (int i = 0; i < SIZE-1; i++)
	{
		if (!keep_idxs[i])
			continue;
		for (int j = i + 1; j < SIZE; j++)
		{
			if (!keep_idxs[j])
				continue;
			double iou = bbox_iou(bboxes[i], bboxes[j]);
			if (iou > nms_th)
			{
				keep_idxs[j] = false;
			}
		}
			
	}

	// Step 4: ɾ�����в�����������bbox
	vector<Rect> bboxes_new;
	vector<double> confs_new;
	for (int i = 0; i < SIZE; i++)
	{
		if (keep_idxs[i])
		{
			bboxes_new.emplace_back(bboxes[i]);
			confs_new.emplace_back(confs[i]);
		}
	}
	bboxes.assign(bboxes_new.begin(), bboxes_new.end());
	confs.assign(confs_new.begin(), confs_new.end());
	return;
}


