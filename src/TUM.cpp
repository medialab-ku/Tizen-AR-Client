#include "TUM.h"
#include<fstream>
#include "FileSystem.h"
using namespace std;

TUM::TUM()
    : seq(0), maxSeq(0)
{

}

void TUM::LoadImages(const std::string &strAssociationFilename, const std::string &strSeqFilename)
{
    ifstream fAssociation;
    fAssociation.open(strAssociationFilename.c_str());
    while(!fAssociation.eof())
    {
        string s;
        getline(fAssociation,s);
        if(!s.empty())
        {
            stringstream ss;
            ss << s;
            double t;
            string sRGB, sD;
            ss >> t;
            timestamps.push_back(t);
            ss >> sRGB;
            ss >> t;
            ss >> sD;

            // cv::Mat rgb, d, rgb_2;
            // rgb = cv::imread("../res/SLAM/" + strSeqFilename + "/" + sRGB, CV_LOAD_IMAGE_UNCHANGED);
            // cv::cvtColor(rgb, rgb_2, CV_BGR2RGB);
		    // d =   cv::imread("../res/SLAM/" + strSeqFilename + "/" + sD,   CV_LOAD_IMAGE_UNCHANGED);
            // imRGB.push_back(rgb_2);
            // imD.push_back(d);    
            // rgbPath.push_back("../res/SLAM/" + strSeqFilename + "/" + sRGB);
            // depthPath.push_back("../res/SLAM/" + strSeqFilename + "/" + sD);
            rgbPath.push_back(FileSystem::GetResourcePath("SLAM/") + strSeqFilename + "/" + sRGB);
            depthPath.push_back(FileSystem::GetResourcePath("SLAM/") + strSeqFilename + "/" + sD);
        }
    }
    maxSeq = rgbPath.size() - 1;
}

void TUM::GetImage(cv::Mat& left, cv::Mat& depth)
{
    if(seq > maxSeq) seq = 0;

    // left = imRGB[seq];
    // depth = imD[seq];
    cv::Mat rgb = cv::imread(rgbPath[seq], CV_LOAD_IMAGE_UNCHANGED);
    cv::cvtColor(rgb, left, CV_BGR2RGB);
    depth = cv::imread(depthPath[seq], CV_LOAD_IMAGE_UNCHANGED);
    seq++;
}
