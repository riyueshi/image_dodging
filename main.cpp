///////////////////////////////////////////////////////////////////////////////
//(you can use the code free, but keep this information in you code, thanks)
//  this code implement a simple image dodging process. which 
//is implement by hist match on light space. firstly, convert 
//the RBG image to HLS image,then implement the hist match on 
//L(light) channel.
//Author: Miller Zhang
//Email: imriyueshi@163.com
//Project Site: 
//Github Link: https://github.com/riyueshi/image_dodging/
///////////////////////////////////////////////////////////////////////////////

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <iostream>
#include <vector>
#include <io.h>
#include <omp.h>

using namespace cv;
using namespace std;


bool normalize_hist(Mat &hist, double pixel_count)
{
	for (size_t i = 0; i < hist.rows; i++)
	{
		hist.at<float>(i, 0) /= pixel_count;
	}
	return true;
}

bool hist_match(const Mat &ref_hist, const Mat &src_hist, Mat &src_image)
{
	vector<double> ref_hist_accumulate(ref_hist.rows);
	vector<double> src_hist_accumulate(src_hist.rows);

	for (size_t i = 0; i < ref_hist.rows; i++)
	{
		if (i == 0)
		{
			ref_hist_accumulate.at(0) = ref_hist.at<float>(0, 0);
		}
		else
		{
			ref_hist_accumulate.at(i) = ref_hist_accumulate.at(i - 1) + ref_hist.at<float>(i, 0);
		}
	}
	for (size_t i = 0; i < src_hist.rows; i++)
	{
		if (i == 0)
		{
			src_hist_accumulate.at(0) = src_hist.at<float>(0, 0);
		}
		else
		{
			src_hist_accumulate.at(i) = src_hist_accumulate.at(i - 1) + src_hist.at<float>(i, 0);
		}
	}

	vector<int> lookup(256);

	/*int j = 0;
	for (int i = 0; i < 256; i++)
	{
		cout << i << endl;
		if (src_hist_accumulate.at(i) < ref_hist_accumulate.at(j) || src_hist_accumulate.at(i) == ref_hist_accumulate.at(j))
		{
			lookup.at(i) = j;
		}
		else
		{
			while (src_hist_accumulate.at(i) > ref_hist_accumulate.at(j))
			{
				j++;
				if (j==256)
				{
					break;
				}
			}
			j = j<255 ? j : 255;
			if (ref_hist_accumulate.at(j)-src_hist_accumulate.at(i)>src_hist_accumulate.at(i)-ref_hist_accumulate.at(j-1))
			{
				lookup.at(i) = j - 1;
			}
			else
			{
				lookup.at(i) = j;
			}
		}
	}*/

	double min_val = 0.0;
	int PG = 0;

	for (int i = 0; i<256; ++i)
	{
		min_val = 1.0;
		for (int j = 0; j<256; ++j)
		{
			if ((ref_hist_accumulate.at(j) - src_hist_accumulate.at(i)) < min_val && (ref_hist_accumulate.at(j) - src_hist_accumulate.at(i)) >= 0)
			{
				min_val = (ref_hist_accumulate.at(j) - src_hist_accumulate.at(i));
				PG = j;
			}

		}
		lookup.at(i) = PG;
	}
	for (size_t i = 0; i < src_image.rows; i++)
	{
		for (size_t j = 0; j < src_image.cols; j++)
		{
			src_image.at<uchar>(i, j) = lookup.at(src_image.at<uchar>(i, j));
		}
	}
	return true;
}

bool get_filelist_from_dir(string path, vector<string>& files)
{
	long   hFile = 0;
	struct _finddata_t fileinfo;
	files.clear();
	if ((hFile = _findfirst(path.c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (!(fileinfo.attrib &  _A_SUBDIR))
				files.push_back(fileinfo.name);
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
		return true;
	}
	else
		return false;
}

bool image_dodging_process(string ref_image_name, string image_type, string src_image_folder, string out_image_folder)
{
	string search_path = src_image_folder + "\\*." + image_type;
	vector<string> image_name_vec;
	if (!get_filelist_from_dir(search_path, image_name_vec))
	{
		cout << "open images failed!" << endl;
		return false;
	}
	vector<string> image_name_src;
	vector<string> image_name_out;
	for (size_t i = 0; i < image_name_vec.size(); i++)
	{
		image_name_src.push_back(src_image_folder + "\\" + image_name_vec.at(i));
		image_name_out.push_back(out_image_folder + "\\" + image_name_vec.at(i));
	}
	Mat ref_image = imread(ref_image_name);
	if (NULL == ref_image.data)
	{
		cout << "open the refence image failed!" << endl;
		return false;
	}
	Mat ref_hls_image;
	cvtColor(ref_image, ref_hls_image, CV_BGR2HLS);

	vector <Mat> ref_hls_channers;
	split(ref_hls_image, ref_hls_channers);
	// Quantize the light to 256 levels
	int lbins = 256;
	int histSize[] = { lbins };
	float lranges[] = { 0, 256 };
	const float* ranges[] = { lranges };
	Mat ref_hist;
	// compute the histogram from the  1-st channels (light)
	int channels_arr[] = { 1 };
	calcHist(&ref_hls_image, 1, channels_arr, Mat(), ref_hist, 1, histSize, ranges, true, false);
	normalize_hist(ref_hist, ref_image.cols*ref_image.rows);

#pragma omp parallel for
	for (int i = 0; i < image_name_src.size(); i++)
	{
		Mat src_image = imread(image_name_src.at(i));
		Mat src_hls_image;
		cvtColor(src_image, src_hls_image, CV_BGR2HLS);
		vector <Mat> src_hls_channels;
		split(src_hls_image, src_hls_channels);
		Mat src_hist;
		calcHist(&src_hls_image, 1, channels_arr, Mat(), src_hist, 1, histSize, ranges, true, false);
		normalize_hist(src_hist, src_image.cols*src_image.rows);
		hist_match(ref_hist, src_hist, src_hls_channels.at(1));
		Mat out_img;
		merge(src_hls_channels, out_img);
		cvtColor(out_img, out_img, CV_HLS2BGR);
		imwrite(image_name_out.at(i), out_img);
	}
	return true;
}

int main(int argc, char** argv)
{
	if (argc!=5)
	{
		cout << "the number of parameters is not correct! we expect 4 parameters." << endl << "press any key to exit" << endl;
		getchar();
		return -1;
	}
	image_dodging_process(argv[1], argv[2], argv[3], argv[4]);

	return 0;
}