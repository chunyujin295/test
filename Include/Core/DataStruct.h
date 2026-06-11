#pragma once

#include <fstream>
#include <vector>
#include <array>
#include <iostream>
#include <Eigen/Dense>
#include <string>
#include <algorithm>
#include "Types.h"

struct ByteCrypt {
    uint16_t kInvalideNum;
    unsigned char SOURCE_XOR_KEY; 

    ByteCrypt() {
        kInvalideNum = 0;
        SOURCE_XOR_KEY = 0xAB; 
    }

    
    bool ReadByteDecrypted(std::ifstream& file, char* data, std::streamsize size) {
        std::vector<char> buffer(size);
        file.read(buffer.data(), size);
        if (file.gcount() != size) {
            return false;
        }
        unsigned char key = SOURCE_XOR_KEY;
        std::transform(buffer.begin(), buffer.end(), data, [key](char c) { return c ^ key; });
        return true;
    };

    void WriteByteDecrypted(std::ofstream& file, const char* data, std::streamsize size) const {
        
        std::vector<char> buffer(data, data + size);
        unsigned char key = SOURCE_XOR_KEY;
        std::transform(buffer.begin(), buffer.end(), buffer.begin(), [key](char c) { return c ^ key; });
        file.write(buffer.data(), size);
    };
};


struct CameraData {
    unsigned int id;
    int width;
    int height;
    int projection_model;
    std::array<double, 12> params; 
    std::string camera_name;
    int cameraModelid;
    int fix_num;
    std::vector<int> fix_param;
    bool hasExtraParam;
    unsigned int group_id;
    std::string group_name;
    double prior_focal;
    double f_35eq;
    double sensorsize;
    std::string num_make;
    std::string num_model;
    std::string num_cameraorientation;
    ByteCrypt byteCrypt;

    CameraData() {
        id = 0;
        width = 0;
        height = 0;
        projection_model = 0;
        camera_name = "";
        params.fill(0.0); 
        cameraModelid = 6;
        fix_num = 0;
        fix_param.clear();
        hasExtraParam = false;
        group_id = -1;
        group_name = "";
        prior_focal = 0.0;
        f_35eq = 0.0;
        sensorsize = 0.0;
        num_make = "";
        num_model = "";
        num_cameraorientation = "";
    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(unsigned int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&width), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&height), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&projection_model), sizeof(int));

        
        unsigned int paramSize = 12;
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&paramSize), sizeof(unsigned int));
        for (int i = 0; i < params.size(); ++i) {
            double tmpData = params[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
        }

        unsigned int camera_name_len = camera_name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&camera_name_len), sizeof(unsigned int));
        byteCrypt.WriteByteDecrypted(out, camera_name.c_str(), camera_name_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&cameraModelid), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&fix_num), sizeof(int));
        for (int i = 0; i < fix_num; ++i) {
            int tmpData = fix_param[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(int));
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasExtraParam), sizeof(bool));
        if (hasExtraParam) {
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&group_id), sizeof(unsigned int));
            unsigned int group_name_len = group_name.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&group_name_len), sizeof(unsigned int));
            byteCrypt.WriteByteDecrypted(out, group_name.c_str(), group_name_len);
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&prior_focal), sizeof(double));
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&f_35eq), sizeof(double));
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&sensorsize), sizeof(double));
            unsigned int num_make_len = num_make.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_make_len), sizeof(unsigned int));
            byteCrypt.WriteByteDecrypted(out, num_make.c_str(), num_make_len);
            unsigned int num_model_len = num_model.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_model_len), sizeof(unsigned int));
            byteCrypt.WriteByteDecrypted(out, num_model.c_str(), num_model_len);
            unsigned int num_cameraorientation_len = num_cameraorientation.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_cameraorientation_len), sizeof(unsigned int));
            byteCrypt.WriteByteDecrypted(out, num_cameraorientation.c_str(), num_cameraorientation_len);
        }
    };
    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(unsigned int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&width), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&height), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&projection_model), sizeof(int));

        
        unsigned int paramSize = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&paramSize), sizeof(unsigned int));
        for (int i = 0; i < params.size(); ++i) {
            double tmpData = 0.0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
            params[i] = tmpData;
        }

        unsigned int camera_name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&camera_name_len), sizeof(unsigned int));
        camera_name.resize(camera_name_len);
        byteCrypt.ReadByteDecrypted(in, &camera_name[0], camera_name_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&cameraModelid), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&fix_num), sizeof(int));
        for (int i = 0; i < fix_num; ++i) {
            int tmpData = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(int));
            fix_param.push_back(tmpData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasExtraParam), sizeof(bool));
        if (hasExtraParam) {
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&group_id), sizeof(unsigned int));
            unsigned int group_name_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&group_name_len), sizeof(unsigned int));
            group_name.resize(group_name_len);
            byteCrypt.ReadByteDecrypted(in, &group_name[0], group_name_len);
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&prior_focal), sizeof(double));
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&f_35eq), sizeof(double));
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&sensorsize), sizeof(double));
            unsigned int num_make_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_make_len), sizeof(unsigned int));
            num_make.resize(num_make_len);
            byteCrypt.ReadByteDecrypted(in, &num_make[0], num_make_len);
            unsigned int num_model_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_model_len), sizeof(unsigned int));
            num_model.resize(num_model_len);
            byteCrypt.ReadByteDecrypted(in, &num_model[0], num_model_len);
            unsigned int num_cameraorientation_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_cameraorientation_len), sizeof(unsigned int));
            num_cameraorientation.resize(num_cameraorientation_len);
            byteCrypt.ReadByteDecrypted(in, &num_cameraorientation[0], num_cameraorientation_len);
        }
    };
};

struct ExifData {
    std::string make;
    std::string model;
    std::string dateTime;
    double focalLength;
    double focalLengthIn35mm;
    double longitude;
    double latitude;
    double altitude;
    bool dewrapflag;
    ByteCrypt byteCrypt;

    ExifData() {
        make = "";
        model = "";
        dateTime = "";
        focalLength = 0.0;
        focalLengthIn35mm = 0.0;
        longitude = 0.0;
        latitude = 0.0;
        altitude = 0.0;
        dewrapflag = false;
    }

    void Serialize(std::ofstream& out) const {
        unsigned int make_len = make.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&make_len), sizeof(make_len));
        byteCrypt.WriteByteDecrypted(out, make.c_str(), make_len);
        unsigned int model_len = model.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&model_len), sizeof(model_len));
        byteCrypt.WriteByteDecrypted(out, model.c_str(), model_len);
        unsigned int dateTime_len = dateTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&dateTime_len), sizeof(dateTime_len));
        byteCrypt.WriteByteDecrypted(out, dateTime.c_str(), dateTime_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&focalLength), sizeof(double));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&focalLengthIn35mm), sizeof(double));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&longitude), sizeof(double));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&latitude), sizeof(double));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&altitude), sizeof(double));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&dewrapflag), sizeof(bool));
    };

    void Deserialize(std::ifstream& in) {
        unsigned int make_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&make_len), sizeof(unsigned int));
        make.resize(make_len);
        byteCrypt.ReadByteDecrypted(in, &make[0], make_len);
        unsigned int model_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&model_len), sizeof(unsigned int));
        model.resize(model_len);
        byteCrypt.ReadByteDecrypted(in, &model[0], model_len);
        unsigned int dateTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&dateTime_len), sizeof(unsigned int));
        dateTime.resize(dateTime_len);
        byteCrypt.ReadByteDecrypted(in, &dateTime[0], dateTime_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&focalLength), sizeof(double));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&focalLengthIn35mm), sizeof(double));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&longitude), sizeof(double));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&latitude), sizeof(double));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&altitude), sizeof(double));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&dewrapflag), sizeof(bool));
       
    };
};

struct ImageData {
    unsigned int image_id;
    unsigned int camera_id;
    long long time;
    int width;
    int height;
    bool dewarp_flag;
    bool hasPosition;
    bool hasPosSigma;
    bool hasRotaiton;
    std::array<double, 3> position;
    std::array<double, 3> pos_sigma;
    std::array<std::array<double, 3>, 3> rotation;
    std::string path;
    std::string name;
    bool isregis;
    bool hasColorParam;
    std::array<double, 3> color_param;
    bool hasCenter;
    std::array<double, 3> center;
    int status;
    bool hasExtraParam;
    std::string num_preview_name_str;
    bool hasPrior;
    unsigned int srs_id_prior;
    std::array<std::array<double, 3>, 3> R_prior;
    std::array<double, 3> center_prior;
    std::array<double, 3> depth;
    ExifData exifData;
    ByteCrypt byteCrypt;

    ImageData() {
        image_id = 0;
        camera_id = 0;
        time = 0;
        width = 0;
        height = 0;
        dewarp_flag = false;
        hasPosition = false;
        hasPosSigma = false;
        hasRotaiton = false;
        position.fill(0.0);
        pos_sigma.fill(0.0);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                rotation[i][j] = 0.0;
            }
        }
        path = "";
        name = "";
        isregis = false;
        hasColorParam = false;
        color_param.fill(0.0);
        hasCenter = false;
        center.fill(0.0);
        status = 0;
        hasExtraParam = false;
        num_preview_name_str = "";
        hasPrior = false;
        srs_id_prior = -1;       
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                R_prior[i][j] = 0.0;
            }
        }
        center_prior.fill(0.0);
        depth.fill(0.0);
    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&image_id), sizeof(unsigned int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&camera_id), sizeof(unsigned int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&time), sizeof(long long));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&width), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&height), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&dewarp_flag), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasPosition), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasPosSigma), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasRotaiton), sizeof(bool));
        if (hasPosition) {
            unsigned int posSize = position.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&posSize), sizeof(unsigned int));
            for (int i = 0; i < posSize; ++i) {
                double tmpData = position[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }
        }
        if (hasPosSigma) {
            unsigned int possigSize = pos_sigma.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&possigSize), sizeof(unsigned int));
            for (int i = 0; i < possigSize; ++i) {
                double tmpData = pos_sigma[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }
        }
        if (hasRotaiton) {
            unsigned int size1 = rotation.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&size1), sizeof(unsigned int));
            for (int i = 0; i < size1; ++i) {
                unsigned int size2 = rotation[i].size();
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&size2), sizeof(unsigned int));
                for (int j = 0; j < size2; ++j) {
                    double tmpData = rotation[i][j];
                    byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
                }
            }
        }


        unsigned int path_len = path.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&path_len), sizeof(path_len));
        byteCrypt.WriteByteDecrypted(out, path.c_str(), path_len);
        unsigned int name_len = name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        byteCrypt.WriteByteDecrypted(out, name.c_str(), name_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&isregis), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasColorParam), sizeof(bool));
        if (hasColorParam) {
            unsigned int colorSize = color_param.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&colorSize), sizeof(unsigned int));
            for (int i = 0; i < colorSize; ++i) {
                double tmpData = color_param[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasCenter), sizeof(bool));
        if (hasCenter) {
            unsigned int centerSize = center.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&centerSize), sizeof(unsigned int));
            for (int i = 0; i < centerSize; ++i) {
                double tmpData = center[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&status), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasExtraParam), sizeof(bool));
        if (hasExtraParam) {
            unsigned int num_preview_name_str_len = num_preview_name_str.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_preview_name_str_len), sizeof(num_preview_name_str_len));
            byteCrypt.WriteByteDecrypted(out, num_preview_name_str.c_str(), num_preview_name_str_len);
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasPrior), sizeof(bool));
            if (hasPrior) {
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&srs_id_prior), sizeof(unsigned int));
                unsigned int size1 = R_prior.size();
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&size1), sizeof(unsigned int));
                for (int i = 0; i < size1; ++i) {
                    unsigned int size2 = R_prior[i].size();
                    byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&size2), sizeof(unsigned int));
                    for (int j = 0; j < size2; ++j) {
                        double tmpData = R_prior[i][j];
                        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
                    }
                }
                unsigned int centerSize = center_prior.size();
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&centerSize), sizeof(unsigned int));
                for (int i = 0; i < centerSize; ++i) {
                    double tmpData = center_prior[i];
                    byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
                }
            }
            unsigned int depthSize = depth.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&depthSize), sizeof(unsigned int));
            for (int i = 0; i < depthSize; ++i) {
                double tmpData = depth[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }
            exifData.Serialize(out);
        }
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&image_id), sizeof(unsigned int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&camera_id), sizeof(unsigned int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&time), sizeof(long long));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&width), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&height), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&dewarp_flag), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasPosition), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasPosSigma), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasRotaiton), sizeof(bool));
        if (hasPosition) {
            unsigned int posSize = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&posSize), sizeof(unsigned int));
            for (int i = 0; i < posSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                position[i] = tmpData;
            }
        }
        if (hasPosSigma) {
            unsigned int possigSize = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&possigSize), sizeof(unsigned int));
            for (int i = 0; i < possigSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                pos_sigma[i] = tmpData;
            }
        }
        if (hasRotaiton) {
            unsigned int size1 = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&size1), sizeof(unsigned int));
            for (int i = 0; i < size1; ++i) {
                unsigned int size2 = 0;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&size2), sizeof(unsigned int));
                for (int j = 0; j < size2; ++j) {
                    double tmpData;
                    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                    rotation[i][j] = tmpData;
                }
            }
        }

        unsigned int path_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&path_len), sizeof(unsigned int));
        path.resize(path_len);
        byteCrypt.ReadByteDecrypted(in, &path[0], path_len);
        unsigned int name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&name_len), sizeof(unsigned int));
        name.resize(name_len);
        byteCrypt.ReadByteDecrypted(in, &name[0], name_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&isregis), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasColorParam), sizeof(bool));

        if (hasColorParam) {
            unsigned int colorSize = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&colorSize), sizeof(unsigned int));
            for (int i = 0; i < colorSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                color_param[i] = tmpData;
            }
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasCenter), sizeof(bool));
        if (hasCenter) {
            unsigned int centerSize = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&centerSize), sizeof(unsigned int));
            for (int i = 0; i < centerSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                center[i] = tmpData;
            }
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&status), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasExtraParam), sizeof(bool));
        if (hasExtraParam) {
            unsigned int num_preview_name_str_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_preview_name_str_len), sizeof(unsigned int));
            num_preview_name_str.resize(num_preview_name_str_len);
            byteCrypt.ReadByteDecrypted(in, &num_preview_name_str[0], num_preview_name_str_len);
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasPrior), sizeof(bool));
            if (hasPrior) {
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&srs_id_prior), sizeof(unsigned int));
                unsigned int size1 = 0;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&size1), sizeof(unsigned int));
                for (int i = 0; i < size1; ++i) {
                    unsigned int size2 = 0;
                    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&size2), sizeof(unsigned int));
                    for (int j = 0; j < size2; ++j) {
                        double tmpData;
                        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                        R_prior[i][j] = tmpData;
                    }
                }
                unsigned int centerSize = 0;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&centerSize), sizeof(unsigned int));
                for (int i = 0; i < centerSize; ++i) {
                    double tmpData;
                    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                    center_prior[i] = tmpData;
                }
            }
            unsigned int depthSize = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&depthSize), sizeof(unsigned int));
            for (int i = 0; i < depthSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                depth[i] = tmpData;
            }
            exifData.Deserialize(in);
        }
    };
};

struct CoordinateData {
    
    int type;
    std::array<double, 3> ori;
    std::string espgStr;
    ByteCrypt byteCrypt;

    CoordinateData() {
        type = 1; 
        ori.fill(0.0); 
        espgStr = "";
    }

    void Serialize(std::ofstream& out) const {
        int typeNum = type;
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&typeNum), sizeof(int));
        if (type == 0) {
        
            int pointSize = 3;
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&pointSize), sizeof(int));
            for (int i = 0; i < pointSize; ++i) {
                double tmpData = ori[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }

        }
        else if (type == 1) {
        

        }else{
            unsigned int espg_len = espgStr.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&espg_len), sizeof(espg_len));
            byteCrypt.WriteByteDecrypted(out, espgStr.c_str(), espg_len);
        }
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&type), sizeof(int));
        
        
        if (type == 0) {
        
            int pointSize;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&pointSize), sizeof(int));
            for (unsigned int i = 0; i < pointSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                ori[i] = tmpData;
            }
        }
        else if (type == 1) {
        

        }
        else 
        {
            unsigned int espg_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&espg_len), sizeof(unsigned int));
            espgStr.resize(espg_len);
            byteCrypt.ReadByteDecrypted(in, &espgStr[0], espg_len);
            espgStr = "EPSG:" + espgStr;
            
        }
    };
};

struct OriBinFile {
    CoordinateData coordinateData;
    std::vector<CameraData> cameraDataVec;
    std::vector<ImageData> imageDataVec;
    unsigned int cameras_size;
    unsigned int images_size;
    ByteCrypt byteCrypt;

    OriBinFile() {
        cameras_size = 0;
        images_size = 0;
        imageDataVec.clear();
        cameraDataVec.clear();
    }

    bool Serialize(std::ofstream& out) const {
        
        const char SOURCE_HEADER_LABEL[] = "SOURCE-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 15); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 15); 

        
        coordinateData.Serialize(out);


        
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&cameras_size), sizeof(unsigned int));
        for (CameraData cameraItem : cameraDataVec)
        {
            cameraItem.Serialize(out);
        }

        
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&images_size), sizeof(unsigned int));

        for (ImageData imageItem : imageDataVec)
        {
            imageItem.Serialize(out);
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[15];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 15);
        const char SOURCE_HEADER_LABEL[] = "SOURCE-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 15)) {
            return false;
        }

        
        coordinateData.Deserialize(in);

        
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&cameras_size), sizeof(unsigned int));  
        for (int i = 0; i < cameras_size; ++i)
        {
            CameraData cameraData;
            cameraData.Deserialize(in);
            cameraDataVec.push_back(cameraData);
        }

        
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&images_size), sizeof(unsigned int));  
        for (int i = 0; i < images_size; ++i)
        {
            ImageData imageData;
            imageData.Deserialize(in);
            imageDataVec.push_back(imageData);
        }
        return true;
    }

};

struct ATSettingData {
    unsigned int keyNum;
    int maxthreads_num;
    int minOverlap;
    int maxOverlap;
    int maxTieptNum;
    int mode;
    
    int ba1_grid_count;
    int ba2_grid_count;
    int max_feature_count_1;
    int max_feature_count_2;
    bool output_tiepoint;
    float max_projection_error;
    
    int reconstruct_mode;
    bool output_rawxml;
    bool use_user_tiepoints;
    
    std::string usertiepoints_path_;
    bool use_gcp;
    std::string control_point_path;
    bool use_constraint;
    std::string constraint_path;
    
    
    
    
    
    
    int tiepoints_policy;
    int pos_policy;
    int ppa_policy;
    int rdis_policy;
    int f_policy;
    int tdis_policy;
    bool use_image_position_;
    std::string image_pos_list;
    bool hasATPath;
    std::string at_path;

    ByteCrypt byteCrypt;


    ATSettingData() {
        keyNum = 20000;
        maxthreads_num = 0;
        minOverlap = 3;
        maxOverlap = 4;
        maxTieptNum = 10000000;
        mode = 0; 
        ba1_grid_count = 20;
        ba2_grid_count = 30;
        max_feature_count_1 = 400;
        max_feature_count_2 = 1000;
        output_tiepoint = true;
        max_projection_error = 3.0f;
        reconstruct_mode = 0; 
        output_rawxml = true;
        use_user_tiepoints = false;
        usertiepoints_path_ = "";
        use_gcp = false;
        control_point_path = "";
        use_constraint = false;
        constraint_path = "";
        tiepoints_policy = 0;
        pos_policy = 0;
        ppa_policy = 0;
        rdis_policy = 0;
        f_policy = 0;
        tdis_policy = 0;
        use_image_position_ = false;
        image_pos_list = "";
        hasATPath = false;
        at_path = "";
    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&keyNum), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&maxthreads_num), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&minOverlap), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&maxOverlap), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&maxTieptNum), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&mode), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&ba1_grid_count), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&ba2_grid_count), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&max_feature_count_1), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&max_feature_count_2), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&output_tiepoint), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&max_projection_error), sizeof(float));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&reconstruct_mode), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&output_rawxml), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&use_user_tiepoints), sizeof(bool));
        
        if (use_user_tiepoints) {
            unsigned int upoint_path_len = usertiepoints_path_.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&upoint_path_len), sizeof(upoint_path_len));
            byteCrypt.WriteByteDecrypted(out, usertiepoints_path_.c_str(), upoint_path_len);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&use_gcp), sizeof(bool));
        if (use_gcp) {
            unsigned int gcp_path_len = control_point_path.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gcp_path_len), sizeof(gcp_path_len));
            byteCrypt.WriteByteDecrypted(out, control_point_path.c_str(), gcp_path_len);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&use_constraint), sizeof(bool));
        if (use_constraint) {
            unsigned int constraint_path_len = constraint_path.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&constraint_path_len), sizeof(constraint_path_len));
            byteCrypt.WriteByteDecrypted(out, constraint_path.c_str(), constraint_path_len);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tiepoints_policy), sizeof(int)); 
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&pos_policy), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&ppa_policy), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&rdis_policy), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&f_policy), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tdis_policy), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&use_image_position_), sizeof(bool));
        if (use_image_position_) {
            unsigned int imagelist_len = image_pos_list.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&imagelist_len), sizeof(imagelist_len));
            byteCrypt.WriteByteDecrypted(out, image_pos_list.c_str(), imagelist_len);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasATPath), sizeof(bool));
        if (hasATPath) {
            unsigned int atpath_len = at_path.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&atpath_len), sizeof(atpath_len));
            byteCrypt.WriteByteDecrypted(out, at_path.c_str(), atpath_len);
        }
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&keyNum), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&maxthreads_num), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&minOverlap), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&maxOverlap), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&maxTieptNum), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&mode), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&ba1_grid_count), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&ba2_grid_count), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&max_feature_count_1), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&max_feature_count_2), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&output_tiepoint), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&max_projection_error), sizeof(float));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&reconstruct_mode), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&output_rawxml), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&use_user_tiepoints), sizeof(bool));
        if (use_user_tiepoints) {
            unsigned int upointpath_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&upointpath_len), sizeof(unsigned int));
            usertiepoints_path_.resize(upointpath_len);
            byteCrypt.ReadByteDecrypted(in, &usertiepoints_path_[0], upointpath_len);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&use_gcp), sizeof(bool));
        if (use_gcp) {
            unsigned int gcp_path_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gcp_path_len), sizeof(unsigned int));
            control_point_path.resize(gcp_path_len);
            byteCrypt.ReadByteDecrypted(in, &control_point_path[0], gcp_path_len);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&use_constraint), sizeof(bool));
        if (use_constraint) {
            unsigned int constraint_path_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&constraint_path_len), sizeof(unsigned int));
            constraint_path.resize(constraint_path_len);
            byteCrypt.ReadByteDecrypted(in, &constraint_path[0], constraint_path_len);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tiepoints_policy), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&pos_policy), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&ppa_policy), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&rdis_policy), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&f_policy), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tdis_policy), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&use_image_position_), sizeof(bool));
        if (use_image_position_) {
            unsigned int imagelist_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&imagelist_len), sizeof(unsigned int));
            image_pos_list.resize(imagelist_len);
            byteCrypt.ReadByteDecrypted(in, &image_pos_list[0], imagelist_len);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasATPath), sizeof(bool));
        if (hasATPath) {
            unsigned int atpath_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&atpath_len), sizeof(unsigned int));
            at_path.resize(atpath_len);
            byteCrypt.ReadByteDecrypted(in, &at_path[0], atpath_len);
        }
    };
};

struct BBoxData {
    bool hasBBbox;
    std::array<double, 3> min;
    std::array<double, 3> max;
    ByteCrypt byteCrypt;

    BBoxData() {
        hasBBbox = false;
        min.fill(0.0);
        max.fill(0.0);
    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasBBbox), sizeof(bool));
        if (hasBBbox) {
            int minpointSize = 3;
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&minpointSize), sizeof(int));
            for (int i = 0; i < minpointSize; ++i) {
                double tmpData = min[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }
            int maxpointSize = 3;
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&maxpointSize), sizeof(int));
            for (int i = 0; i < maxpointSize; ++i) {
                double tmpData = max[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }

        }
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasBBbox), sizeof(bool));
        if (hasBBbox) {
            int minpointSize;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&minpointSize), sizeof(int));
            for (unsigned int i = 0; i < minpointSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                min[i] = tmpData;
            }
            int maxpointSize;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&maxpointSize), sizeof(int));
            for (unsigned int i = 0; i < maxpointSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                max[i] = tmpData;
            }
        }
    };
};

struct TileData {
    int index;
    std::string name;
    int status;
    bool isempty;
    bool hasBBbox;
    BBoxData bbox;
    unsigned int imageNum;
    std::vector<unsigned int> imageids;
    ByteCrypt byteCrypt;

    TileData() {
        index = 0;
        name = "";
        status = 0;
        isempty = false;
        hasBBbox = false;
        imageNum = 0;
        imageids.clear();
    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&index), sizeof(int));
        unsigned int name_len = name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        byteCrypt.WriteByteDecrypted(out, name.c_str(), name_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&status), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&isempty), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasBBbox), sizeof(bool));
        if (hasBBbox) {
            bbox.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&imageNum), sizeof(unsigned int));
        for (int i = 0; i < imageNum; ++i) {
            unsigned int tmpData = imageids[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(unsigned int));
        }
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&index), sizeof(int));
        unsigned int name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&name_len), sizeof(unsigned int));
        name.resize(name_len);
        byteCrypt.ReadByteDecrypted(in, &name[0], name_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&status), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&isempty), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasBBbox), sizeof(bool));
        if (hasBBbox) {
            bbox.Deserialize(in);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&imageNum), sizeof(unsigned int));
        for (int i = 0; i < imageNum; ++i) {
            unsigned int tmpData;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(unsigned int));
            imageids.push_back(tmpData);
        }
    };
};

struct SRSData {
    bool hasSRS;
    std::string definition;
    std::array<double, 3> ori;

    ByteCrypt byteCrypt;

    SRSData() {
        hasSRS = false;
        definition = "";
        ori.fill(0.0);
    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasSRS), sizeof(bool));
        if (hasSRS) {
            unsigned int definition_len = definition.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&definition_len), sizeof(definition_len));
            byteCrypt.WriteByteDecrypted(out, definition.c_str(), definition_len);
            for (int i = 0; i < 3; ++i) {
                double tmpData = ori[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }
        }
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasSRS), sizeof(bool));
        if (hasSRS) {
            unsigned int definition_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&definition_len), sizeof(unsigned int));
            definition.resize(definition_len);
            byteCrypt.ReadByteDecrypted(in, &definition[0], definition_len);
            for (int i = 0; i < 3; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                ori[i] = (tmpData);
            }
        }
    };
};

struct ProductionData {
    int id;
    std::string name;
    std::string modelingsettings;    
    std::string production_format;
    SRSData srsData;
    std::string destination;
    unsigned int tileSize;
    std::vector<std::string> tiles;


    ByteCrypt byteCrypt;

    ProductionData() {
        id = 0;
        name = "";
        modelingsettings = "";
        production_format = "";
        destination = "";
        tileSize = 0;
        tiles.clear();

    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(int));
        unsigned int name_len = name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        byteCrypt.WriteByteDecrypted(out, name.c_str(), name_len);
        unsigned int setting_len = modelingsettings.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&setting_len), sizeof(setting_len));
        byteCrypt.WriteByteDecrypted(out, modelingsettings.c_str(), setting_len);
        unsigned int format_len = production_format.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&format_len), sizeof(format_len));
        byteCrypt.WriteByteDecrypted(out, production_format.c_str(), format_len);
        srsData.Serialize(out);
        unsigned int dest_len = destination.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&dest_len), sizeof(dest_len));
        byteCrypt.WriteByteDecrypted(out, destination.c_str(), dest_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tileSize), sizeof(unsigned int));
        for (int i = 0; i < tileSize; ++i) {
            std::string tmpData = tiles[i];
            unsigned int tmp_len = tmpData.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp_len), sizeof(tmp_len));
            byteCrypt.WriteByteDecrypted(out, tmpData.c_str(), tmp_len);
        }
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(int));
        unsigned int name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&name_len), sizeof(unsigned int));
        name.resize(name_len);
        byteCrypt.ReadByteDecrypted(in, &name[0], name_len);
        unsigned int setting_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&setting_len), sizeof(unsigned int));
        modelingsettings.resize(setting_len);
        byteCrypt.ReadByteDecrypted(in, &modelingsettings[0], setting_len);
        unsigned int format_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&format_len), sizeof(unsigned int));
        production_format.resize(format_len);
        byteCrypt.ReadByteDecrypted(in, &production_format[0], format_len);
        srsData.Deserialize(in);
        unsigned int dest_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&dest_len), sizeof(unsigned int));
        destination.resize(dest_len);
        byteCrypt.ReadByteDecrypted(in, &destination[0], dest_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tileSize), sizeof(unsigned int));
        tiles.clear();
        for (int i = 0; i < tileSize; ++i) {
            std::string tmpData;
            unsigned int tmp_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp_len), sizeof(unsigned int));
            tmpData.resize(tmp_len);
            byteCrypt.ReadByteDecrypted(in, &tmpData[0], tmp_len);
            tiles.push_back(tmpData);
        }
    };
};

struct TillData {
    int tiling_mode;
    float expected_max_ram_used;
    std::array<double, 3> automatic_origin;
    std::array<double, 3> custom_origin;
    float tileSize;
    ByteCrypt byteCrypt;

    TillData() {
        tiling_mode = 0; 
        expected_max_ram_used = 0.0;
        automatic_origin.fill(0.0);
        custom_origin.fill(0.0);
        tileSize = -1.;
    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tiling_mode), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&expected_max_ram_used), sizeof(float));
        if (tiling_mode == 0) {
        
           
        }
        else {
            int auto_pointSize = 3;
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&auto_pointSize), sizeof(int));
            for (int i = 0; i < auto_pointSize; ++i) {
                double tmpData = automatic_origin[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }
            int cust_pointSize = 3;
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&cust_pointSize), sizeof(int));
            for (int i = 0; i < cust_pointSize; ++i) {
                double tmpData = custom_origin[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
            }
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tileSize), sizeof(float));
        }
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tiling_mode), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&expected_max_ram_used), sizeof(float));
        if (tiling_mode == 0) {
        

        }
        else {
            int auto_pointSize;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&auto_pointSize), sizeof(int));
            for (unsigned int i = 0; i < auto_pointSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                automatic_origin[i] = tmpData;
            }
            int cust_pointSize;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&cust_pointSize), sizeof(int));
            for (unsigned int i = 0; i < cust_pointSize; ++i) {
                double tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                custom_origin[i] = tmpData;
            }
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tileSize), sizeof(float));
        }
    };
};

struct ReconstrutionData {
    unsigned int id;
    std::string name;
    bool hasCoord;
    CoordinateData coordinateData;
    BBoxData boundingbox_custom;
    bool hasBoundary;
    std::vector < std::vector< std::vector<double> > >  boundary_custom_;
    int boundary_level1_size;
    int boundary_level2_size;
    TillData tillData;
    int tileNum;   
    std::vector<TileData> tileVec;
    int productionNum;
    std::vector<ProductionData> productionVec;
    int Geometric_Level;
    bool ColorBalanced;
    int Untexture_Fill_Mode;
    bool DiscardEmptyTiles;
    int HoleFillingMode;
    std::array<int, 3> Texture_Fill_Color;

    ByteCrypt byteCrypt;

    ReconstrutionData() {
        id = 0;
        name = ""; 
        hasCoord = false;
        hasBoundary = false;
        boundary_level1_size = 0;
        boundary_level2_size = 0;
        tileNum = 0;       
        tileVec.clear();
        productionNum = 0;
        productionVec.clear();
        Geometric_Level = 1;  
        ColorBalanced = true;
        Untexture_Fill_Mode = 1; 
        DiscardEmptyTiles = true;
        HoleFillingMode = 1; 
        Texture_Fill_Color.fill(128);
    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(int));
        unsigned int name_len = name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        byteCrypt.WriteByteDecrypted(out, name.c_str(), name_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasCoord), sizeof(bool));
        if (hasCoord) {
            coordinateData.Serialize(out);
        }
        boundingbox_custom.Serialize(out);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasBoundary), sizeof(bool));
        
        if (hasBoundary) {
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&boundary_level1_size), sizeof(int));
            for (int i = 0; i < boundary_level1_size; ++i) {
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&boundary_level2_size), sizeof(int));
                for (int j = 0; j < boundary_level2_size; ++j) {
                    int boundary_level3_size = 2;
                    byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&boundary_level3_size), sizeof(int));
                    for (int k = 0; k < boundary_level3_size; ++k) {
                        double tmpData = boundary_custom_[i][j][k];
                        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
                    }
                }
            }
            
        }
        tillData.Serialize(out);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tileNum), sizeof(int));
        for (int i = 0; i < tileNum; ++i) {
            tileVec[i].Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&productionNum), sizeof(int));
        for (int i = 0; i < productionNum; ++i) {
            productionVec[i].Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&Geometric_Level), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&ColorBalanced), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&Untexture_Fill_Mode), sizeof(int));
        if (Untexture_Fill_Mode == 0) {
        
            int fillSize = 3;
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&fillSize), sizeof(int));
            for (int i = 0; i < fillSize; ++i) {
                int tmpData = Texture_Fill_Color[i];
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(int));
            }
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&DiscardEmptyTiles), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&HoleFillingMode), sizeof(int));
        
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(int));
        unsigned int name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&name_len), sizeof(unsigned int));
        name.resize(name_len);
        byteCrypt.ReadByteDecrypted(in, &name[0], name_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasCoord), sizeof(bool));
        if (hasCoord) {
            coordinateData.Deserialize(in);
        }
        boundingbox_custom.Deserialize(in);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasBoundary), sizeof(bool));
        
        if (hasBoundary) {
            boundary_custom_.clear();
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&boundary_level1_size), sizeof(int));
            for (int i = 0; i < boundary_level1_size; ++i) {
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&boundary_level2_size), sizeof(int));
                std::vector< std::vector<double> > thirdLeve2;
                for (int j = 0; j < boundary_level2_size; ++j) {
                    int boundary_level3_size = 0;
                    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&boundary_level3_size), sizeof(int));
                    std::vector<double> thirdLevel;
                    for (int k = 0; k < boundary_level3_size; ++k) {
                        double tmpData;
                        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                        thirdLevel.push_back(tmpData);
                    }
                    thirdLeve2.push_back(thirdLevel);
                }
                boundary_custom_.push_back(thirdLeve2);
            }

        }
        tillData.Deserialize(in);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tileNum), sizeof(int));
        for (int i = 0; i < tileNum; ++i) {
            TileData tmpData;
            tmpData.Deserialize(in);
            tileVec.push_back(tmpData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&productionNum), sizeof(int));
        for (int i = 0; i < productionNum; ++i) {
            ProductionData productionData;
            productionData.Deserialize(in);
            productionVec.push_back(productionData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&Geometric_Level), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&ColorBalanced), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&Untexture_Fill_Mode), sizeof(int));
        
        if (Untexture_Fill_Mode == 0) {
            int fillSize;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&fillSize), sizeof(int));
            for (unsigned int i = 0; i < fillSize; ++i) {
                int tmpData;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(int));
                Texture_Fill_Color[i] = tmpData;
            }

        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&DiscardEmptyTiles), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&HoleFillingMode), sizeof(int));
    };
};

struct BLKBinFile {
    int    gen_block_task_category = 0;     // 0=重建(默认), 1=生成式
    int    gen_next_generation_id = 1;      // 递增计数器 (只增不复用)
    std::string gen_params_json;            // 生成式参数 JSON 字符串
    std::string gen_info_json;              // generations_info_ JSON 数组字符串 (新增)
    int    genJobNum = 0;                   // generationjobs_ 条目数 (对标 jobNum)
    std::vector<std::string> genJobVec;     // "task_uuid:job_name" (对标 jobVec)

    std::string blkString;
    std::string mergedFrom;
    int blkId;
    std::string job;
    bool isFinished;
    bool btoPredict;
    int AT_Num;
    std::string BlockXML;
    std::string Tiepoints;
    bool hasAT;
    bool hasGCP;
    std::string ATJson;
    std::string GCPJson;
    int tiepointNum;
    ATSettingData atSetting;
    int reconstructionNum;
    std::vector<ReconstrutionData> reconstrutionDataVec;
    int jobNum;
    std::vector<std::string> jobVec;

    ByteCrypt byteCrypt;

    BLKBinFile() {
        gen_block_task_category = 0;// default: rebuild
        gen_params_json = "";
        gen_info_json = "";
        genJobNum = 0;
        genJobVec.clear();

        blkString = "";
        mergedFrom = "";
        blkId = 0;
        job = "";
        isFinished = false;
        btoPredict = false;
        AT_Num = 0;
        BlockXML = "";
        Tiepoints = "";
        hasAT = false;
        hasGCP = false;
        ATJson = "";
        GCPJson = "";
        tiepointNum = 0;
        reconstructionNum = 0;
        reconstrutionDataVec.clear();
        jobNum = 0;
        jobVec.clear();
    }

    bool Serialize(std::ofstream& out) const {
        
        const char SOURCE_HEADER_LABEL[] = "BBLK-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 13); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 13); 

        

        unsigned int blkStr_len = blkString.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&blkStr_len), sizeof(blkStr_len));
        byteCrypt.WriteByteDecrypted(out, blkString.c_str(), blkStr_len);
        unsigned int mergedFrom_len = mergedFrom.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&mergedFrom_len), sizeof(mergedFrom_len));
        byteCrypt.WriteByteDecrypted(out, mergedFrom.c_str(), mergedFrom_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&blkId), sizeof(int));
        unsigned int job_len = job.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&job_len), sizeof(job_len));
        byteCrypt.WriteByteDecrypted(out, job.c_str(), job_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&isFinished), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&btoPredict), sizeof(bool));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&AT_Num), sizeof(int));
        unsigned int BlockXML_len = BlockXML.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&BlockXML_len), sizeof(BlockXML_len));
        byteCrypt.WriteByteDecrypted(out, BlockXML.c_str(), BlockXML_len);
        unsigned int Tiepoints_len = Tiepoints.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&Tiepoints_len), sizeof(Tiepoints_len));
        byteCrypt.WriteByteDecrypted(out, Tiepoints.c_str(), Tiepoints_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasAT), sizeof(bool));
        if (hasAT) {
            unsigned int ATJson_len = ATJson.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&ATJson_len), sizeof(ATJson_len));
            byteCrypt.WriteByteDecrypted(out, ATJson.c_str(), ATJson_len);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasGCP), sizeof(bool));
        if (hasGCP) {
            unsigned int GCPJson_len = GCPJson.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&GCPJson_len), sizeof(GCPJson_len));
            byteCrypt.WriteByteDecrypted(out, GCPJson.c_str(), GCPJson_len);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tiepointNum), sizeof(int));
        
        atSetting.Serialize(out);
        
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&reconstructionNum), sizeof(int));
        for (unsigned int i = 0; i < reconstructionNum; ++i)
        {
            reconstrutionDataVec[i].Serialize(out);
        }
        
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&jobNum), sizeof(int));
        for (unsigned int i = 0; i < jobNum; ++i)
        {
            std::string tmpJob = jobVec[i];
            unsigned int tmpJob_len = tmpJob.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpJob_len), sizeof(tmpJob_len));
            byteCrypt.WriteByteDecrypted(out, tmpJob.c_str(), tmpJob_len);
        }

        // generate -begin
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_block_task_category), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_next_generation_id), sizeof(int));

        unsigned int gen_params_json_len = gen_params_json.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_params_json_len), sizeof(gen_params_json_len));
        byteCrypt.WriteByteDecrypted(out, gen_params_json.c_str(), gen_params_json_len);

        unsigned int gen_info_json_len = gen_info_json.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_info_json_len), sizeof(gen_info_json_len));
        byteCrypt.WriteByteDecrypted(out, gen_info_json.c_str(), gen_info_json_len);

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&genJobNum), sizeof(int));
        for (int i = 0; i < genJobNum; i++) {
            unsigned int len = genJobVec[i].size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&len), sizeof(unsigned int));
            byteCrypt.WriteByteDecrypted(out, genJobVec[i].c_str(), len);
        }
        // generate -end
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[13];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 13);
        const char SOURCE_HEADER_LABEL[] = "BBLK-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 13)) {
            return false;
        }

        unsigned int blkStr_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&blkStr_len), sizeof(unsigned int));
        blkString.resize(blkStr_len);
        byteCrypt.ReadByteDecrypted(in, &blkString[0], blkStr_len);
        unsigned int mergedFrom_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&mergedFrom_len), sizeof(unsigned int));
        mergedFrom.resize(mergedFrom_len);
        byteCrypt.ReadByteDecrypted(in, &mergedFrom[0], mergedFrom_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&blkId), sizeof(int));
        unsigned int job_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&job_len), sizeof(unsigned int));
        job.resize(job_len);
        byteCrypt.ReadByteDecrypted(in, &job[0], job_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&isFinished), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&btoPredict), sizeof(bool));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&AT_Num), sizeof(int));
        unsigned int BlockXML_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&BlockXML_len), sizeof(unsigned int));
        BlockXML.resize(BlockXML_len);
        byteCrypt.ReadByteDecrypted(in, &BlockXML[0], BlockXML_len);
        unsigned int Tiepoints_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&Tiepoints_len), sizeof(unsigned int));
        Tiepoints.resize(Tiepoints_len);
        byteCrypt.ReadByteDecrypted(in, &Tiepoints[0], Tiepoints_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasAT), sizeof(bool));
        if (hasAT) {
            unsigned int ATJson_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&ATJson_len), sizeof(unsigned int));
            ATJson.resize(ATJson_len);
            byteCrypt.ReadByteDecrypted(in, &ATJson[0], ATJson_len);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasGCP), sizeof(bool));
        if (hasGCP) {
            unsigned int GCPJson_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&GCPJson_len), sizeof(unsigned int));
            GCPJson.resize(GCPJson_len);
            byteCrypt.ReadByteDecrypted(in, &GCPJson[0], GCPJson_len);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tiepointNum), sizeof(int));
        atSetting.Deserialize(in);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&reconstructionNum), sizeof(int));
        reconstrutionDataVec.clear();
        for (unsigned int i = 0; i < reconstructionNum; ++i)
        {
            ReconstrutionData reconstructionData;
            reconstructionData.Deserialize(in);
            reconstrutionDataVec.push_back(reconstructionData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&jobNum), sizeof(int));
        for (unsigned int i = 0; i < jobNum; ++i)
        {
            std::string jobStr = "";
            unsigned int job_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&job_len), sizeof(unsigned int));
            jobStr.resize(job_len);
            byteCrypt.ReadByteDecrypted(in, &jobStr[0], job_len);
            jobVec.push_back(jobStr);
        }

        // GenTask fields — absent in legacy block bins written before cyj-forDebug.
        gen_block_task_category = 0;
        gen_params_json.clear();
        gen_info_json.clear();
        genJobNum = 0;
        genJobVec.clear();
        if (in.good() && in.peek() != std::char_traits<char>::eof()) {
        // generate -begin
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_block_task_category), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_next_generation_id), sizeof(int));

            unsigned int gen_params_json_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_params_json_len), sizeof(unsigned int));
            gen_params_json.resize(gen_params_json_len);
            if (gen_params_json_len > 0) {
                byteCrypt.ReadByteDecrypted(in, &gen_params_json[0], gen_params_json_len);
            }

            unsigned int gen_info_json_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_info_json_len), sizeof(unsigned int));
            gen_info_json.resize(gen_info_json_len);
            if (gen_info_json_len > 0) {
                byteCrypt.ReadByteDecrypted(in, &gen_info_json[0], gen_info_json_len);
            }

            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&genJobNum), sizeof(int));
            genJobVec.resize(genJobNum);
            for (int i = 0; i < genJobNum; i++) {
                unsigned int len = 0;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&len), sizeof(unsigned int));
                genJobVec[i].resize(len);
                if (len > 0) {
                    byteCrypt.ReadByteDecrypted(in, &genJobVec[i][0], len);
                }
            }
        }
        return true;
    }
};

struct RunData {
    std::string runHostName;
    std::string runUserName;
    std::string runStartTime;
    std::string runEndTime;
    ByteCrypt byteCrypt;

    RunData() {
        runHostName = "";
        runUserName = "";
        runStartTime = "";
        runEndTime = "";
    }

    void Serialize(std::ofstream& out) const {
        unsigned int runHostName_len = runHostName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&runHostName_len), sizeof(runHostName_len));
        byteCrypt.WriteByteDecrypted(out, runHostName.c_str(), runHostName_len);
        unsigned int runUserName_len = runUserName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&runUserName_len), sizeof(runUserName_len));
        byteCrypt.WriteByteDecrypted(out, runUserName.c_str(), runUserName_len);
        unsigned int runStartTime_len = runStartTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&runStartTime_len), sizeof(runStartTime_len));
        byteCrypt.WriteByteDecrypted(out, runStartTime.c_str(), runStartTime_len);
        unsigned int runEndTime_len = runEndTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&runEndTime_len), sizeof(runEndTime_len));
        byteCrypt.WriteByteDecrypted(out, runEndTime.c_str(), runEndTime_len);
    };

    void Deserialize(std::ifstream& in) {
        unsigned int runHostName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&runHostName_len), sizeof(unsigned int));
        runHostName.resize(runHostName_len);
        byteCrypt.ReadByteDecrypted(in, &runHostName[0], runHostName_len);
        unsigned int runUserName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&runUserName_len), sizeof(unsigned int));
        runUserName.resize(runUserName_len);
        byteCrypt.ReadByteDecrypted(in, &runUserName[0], runUserName_len);
        unsigned int runStartTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&runStartTime_len), sizeof(unsigned int));
        runStartTime.resize(runStartTime_len);
        byteCrypt.ReadByteDecrypted(in, &runStartTime[0], runStartTime_len);
        unsigned int runEndTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&runEndTime_len), sizeof(unsigned int));
        runEndTime.resize(runEndTime_len);
        byteCrypt.ReadByteDecrypted(in, &runEndTime[0], runEndTime_len);
    };
};

struct RunInfoData {
    std::string submitHostName;
    std::string submitUser;
    std::string submitTime;
    std::string submitEndTime;
    RunData runData;
    ByteCrypt byteCrypt;

    RunInfoData() {
        submitHostName = "";
        submitUser = "";
        submitTime = "";
        submitEndTime = "";
    }

    void Serialize(std::ofstream& out) const {
        unsigned int submitHostName_len = submitHostName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&submitHostName_len), sizeof(submitHostName_len));
        byteCrypt.WriteByteDecrypted(out, submitHostName.c_str(), submitHostName_len);
        unsigned int submitUser_len = submitUser.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&submitUser_len), sizeof(submitUser_len));
        byteCrypt.WriteByteDecrypted(out, submitUser.c_str(), submitUser_len);
        unsigned int submitTime_len = submitTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&submitTime_len), sizeof(submitTime_len));
        byteCrypt.WriteByteDecrypted(out, submitTime.c_str(), submitTime_len);
        unsigned int submitEndTime_len = submitEndTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&submitEndTime_len), sizeof(submitEndTime_len));
        byteCrypt.WriteByteDecrypted(out, submitEndTime.c_str(), submitEndTime_len);
        runData.Serialize(out);
    };

    void Deserialize(std::ifstream& in) {
        unsigned int submitHostName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&submitHostName_len), sizeof(unsigned int));
        submitHostName.resize(submitHostName_len);
        byteCrypt.ReadByteDecrypted(in, &submitHostName[0], submitHostName_len);
        unsigned int submitUser_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&submitUser_len), sizeof(unsigned int));
        submitUser.resize(submitUser_len);
        byteCrypt.ReadByteDecrypted(in, &submitUser[0], submitUser_len);
        unsigned int submitTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&submitTime_len), sizeof(unsigned int));
        submitTime.resize(submitTime_len);
        byteCrypt.ReadByteDecrypted(in, &submitTime[0], submitTime_len);
        unsigned int submitEndTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&submitEndTime_len), sizeof(unsigned int));
        submitEndTime.resize(submitEndTime_len);
        byteCrypt.ReadByteDecrypted(in, &submitEndTime[0], submitEndTime_len);
        runData.Deserialize(in);
    };
};

struct TaskInfoData {
    int id;
    int type;
    int status;
    std::string functionName;
    std::string startTime;
    std::string endTime;
    ByteCrypt byteCrypt;

    TaskInfoData() {
        id = -1;
        type = -1;
        status = 0;
        functionName = "";
        startTime = "";
        endTime = "";
    }

    void Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&type), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&status), sizeof(int));
        unsigned int functionName_len = functionName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&functionName_len), sizeof(functionName_len));
        byteCrypt.WriteByteDecrypted(out, functionName.c_str(), functionName_len);
        unsigned int startTime_len = startTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&startTime_len), sizeof(startTime_len));
        byteCrypt.WriteByteDecrypted(out, startTime.c_str(), startTime_len);
        unsigned int endTime_len = endTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&endTime_len), sizeof(endTime_len));
        byteCrypt.WriteByteDecrypted(out, endTime.c_str(), endTime_len);
    };

    void Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&type), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&status), sizeof(int));
        unsigned int functionName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&functionName_len), sizeof(unsigned int));
        functionName.resize(functionName_len);
        byteCrypt.ReadByteDecrypted(in, &functionName[0], functionName_len);
        unsigned int startTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&startTime_len), sizeof(unsigned int));
        startTime.resize(startTime_len);
        byteCrypt.ReadByteDecrypted(in, &startTime[0], startTime_len);
        unsigned int endTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&endTime_len), sizeof(unsigned int));
        endTime.resize(endTime_len);
        byteCrypt.ReadByteDecrypted(in, &endTime[0], endTime_len);
    };
};

struct TaskTimeFile {
    RunInfoData runInfoData;
    int taskNum;
    std::vector<TaskInfoData> taskVec;
    ByteCrypt byteCrypt;

    TaskTimeFile() {
        taskNum = 0;
        taskVec.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "TTIME-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 14); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 14); 

        runInfoData.Serialize(out);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&taskNum), sizeof(int));
        for (unsigned int i = 0; i < taskNum; ++i)
        {
            taskVec[i].Serialize(out);
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[14];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 14);
        const char SOURCE_HEADER_LABEL[] = "TTIME-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 14)) {
            return false;
        }

        runInfoData.Deserialize(in);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&taskNum), sizeof(int));
        for (unsigned int i = 0; i < taskNum; ++i)
        {
            TaskInfoData taskInfoData;
            taskInfoData.Deserialize(in);
            taskVec.push_back(taskInfoData);
        }
        return true;
    };
};

struct FeedBackData {
    int status;
    float percent;
    int taskRetVal;
    std::string msg;
    ByteCrypt byteCrypt;

    FeedBackData() {
        status = 0;
        percent = 0.0f;
        taskRetVal = -1;
        msg = "";
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&status), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&percent), sizeof(float));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&taskRetVal), sizeof(int));
        unsigned int msg_len = msg.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&msg_len), sizeof(msg_len));
        byteCrypt.WriteByteDecrypted(out, msg.c_str(), msg_len);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&status), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&percent), sizeof(float));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&taskRetVal), sizeof(int));
        unsigned int msg_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&msg_len), sizeof(unsigned int));
        msg.resize(msg_len);
        byteCrypt.ReadByteDecrypted(in, &msg[0], msg_len);
        return true;
    };
};

struct FeedBackFile {
    FeedBackData feedBackData;
    ByteCrypt byteCrypt;

    FeedBackFile() {

    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "FEED-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 13); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 13); 

        feedBackData.Serialize(out);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[13];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 13);
        const char SOURCE_HEADER_LABEL[] = "FEED-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 13)) {
            return false;
        }

        feedBackData.Deserialize(in);
        return true;
    };
};

struct JobInfoData {
    std::string projectPath;
    std::string itemPath;

    std::string freeze_no;
    int frozen_points;
    int consumed;
    int refunded;
    int total_balance;
    int available_points;
    bool points_settled;

    ByteCrypt byteCrypt;

    JobInfoData() {
        projectPath = "";
        itemPath = "";
    }

    bool Serialize(std::ofstream& out) const {
        unsigned int projectPath_len = projectPath.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&projectPath_len), sizeof(projectPath_len));
        byteCrypt.WriteByteDecrypted(out, projectPath.c_str(), projectPath_len);
        unsigned int jobName_len = itemPath.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&jobName_len), sizeof(jobName_len));
        byteCrypt.WriteByteDecrypted(out, itemPath.c_str(), jobName_len);

        auto ws = [&](const std::string& s) {
            unsigned int len = s.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&len), sizeof(len));
            byteCrypt.WriteByteDecrypted(out, s.c_str(), len);
        };
        auto wi = [&](int v) {
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&v), sizeof(int));
        };
        ws(freeze_no);
        wi(frozen_points); wi(consumed); wi(refunded);
        wi(total_balance); wi(available_points); wi(points_settled ? 1 : 0);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        unsigned int projectPath_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&projectPath_len), sizeof(unsigned int));
        projectPath.resize(projectPath_len);
        byteCrypt.ReadByteDecrypted(in, &projectPath[0], projectPath_len);
        unsigned int itemPath_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&itemPath_len), sizeof(unsigned int));
        itemPath.resize(itemPath_len);
        byteCrypt.ReadByteDecrypted(in, &itemPath[0], itemPath_len);

        auto rs = [&](std::string& s) {
            unsigned int len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&len), sizeof(unsigned int));
            s.resize(len);
            byteCrypt.ReadByteDecrypted(in, &s[0], len);
        };
        auto ri = [&](int& v) {
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&v), sizeof(int));
        };

        rs(freeze_no);
        ri(frozen_points); ri(consumed); ri(refunded);
        ri(total_balance); ri(available_points);
        int settledInt = 0; ri(settledInt); points_settled = (settledInt != 0);

        return true;
    };
};

struct TaskItemData {
    std::string msg;
    float percent;
    int status;
    int type;
    std::string  projectPath;
    std::string itemPath;
    int id;
    int fatherId;
    int dependNum;
    std::set<int> depends;
    RunData runData;
    ByteCrypt byteCrypt;

    TaskItemData() {
        msg = "";
        percent = 0.0f;
        status = 0;
        type = 1;
        projectPath = "";
        itemPath = "";
        id = -1;
        fatherId = -1;
        dependNum = 0;
        depends.clear();
        
    }

    bool Serialize(std::ofstream& out) const {
        unsigned int msg_len = msg.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&msg_len), sizeof(msg_len));
        byteCrypt.WriteByteDecrypted(out, msg.c_str(), msg_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&percent), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&status), sizeof(float));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&type), sizeof(int));
        unsigned int projectPath_len = projectPath.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&projectPath_len), sizeof(projectPath_len));
        byteCrypt.WriteByteDecrypted(out, projectPath.c_str(), projectPath_len);
        unsigned int itemPath_len = itemPath.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&itemPath_len), sizeof(itemPath_len));
        byteCrypt.WriteByteDecrypted(out, itemPath.c_str(), itemPath_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&fatherId), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&dependNum), sizeof(int));
        for (int elem : depends) {
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&elem), sizeof(int));
        }
        runData.Serialize(out);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        unsigned int msg_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&msg_len), sizeof(unsigned int));
        msg.resize(msg_len);
        byteCrypt.ReadByteDecrypted(in, &msg[0], msg_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&percent), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&status), sizeof(float));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&type), sizeof(int));
        unsigned int projectPath_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&projectPath_len), sizeof(unsigned int));
        projectPath.resize(projectPath_len);
        byteCrypt.ReadByteDecrypted(in, &projectPath[0], projectPath_len);
        unsigned int itemPath_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&itemPath_len), sizeof(unsigned int));
        itemPath.resize(itemPath_len);
        byteCrypt.ReadByteDecrypted(in, &itemPath[0], itemPath_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&fatherId), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&dependNum), sizeof(int));
        for (int i = 0; i < dependNum; i++) {
            int tmp = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(int));
            depends.insert(tmp);
        }
        runData.Deserialize(in);
        return true;
    };
};




struct JobListFile {
    std::string jobName;
    JobInfoData jobInfoData;
    RunInfoData runInfoData;
    FeedBackData feedBackData;
    int taskNum;
    std::vector<TaskItemData> taskVec;
    ByteCrypt byteCrypt;

    JobListFile() {
        jobName = "";
        taskNum = 0;
        taskVec.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "JLIST-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 14); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 14); 
       
        unsigned int jobName_len = jobName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&jobName_len), sizeof(jobName_len));
        byteCrypt.WriteByteDecrypted(out, jobName.c_str(), jobName_len);
        jobInfoData.Serialize(out);
        runInfoData.Serialize(out);
        feedBackData.Serialize(out);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&taskNum), sizeof(int));
        for (int i = 0; i < taskNum; i++)
        {
            taskVec[i].Serialize(out);
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[14];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 14);
        const char SOURCE_HEADER_LABEL[] = "JLIST-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 14)) {
            return false;
        }

        unsigned int jobName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&jobName_len), sizeof(unsigned int));
        jobName.resize(jobName_len);
        byteCrypt.ReadByteDecrypted(in, &jobName[0], jobName_len);
        jobInfoData.Deserialize(in);
        runInfoData.Deserialize(in);
        feedBackData.Deserialize(in);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&taskNum), sizeof(int));
        for (int i = 0; i < taskNum; i++)
        {
            TaskItemData taskItemData;
            taskItemData.Deserialize(in);
            taskVec.push_back(taskItemData);
        }
        return true;
    };
};

struct TaskMetaData {
    int id;
    std::string msg;
    std::string name;
    float begin;
    float end;
    int fatherId;
    int match_id;
    int type;
    int imagNum;
    std::vector<int> imgIds;
    int dependNum;
    std::vector<int> depends;
    std::string functionName;
    int sfmmem;
    int sfmId;
    int keyMaxImgNum;
    int matchMaxImgNum;
    int sfm_task_num;
    int match_task_num;
    int matchIdNum;
    std::vector<int> matchIds;
    ByteCrypt byteCrypt;

    TaskMetaData() {
        id = 0;
        msg = "";
        name = "";
        begin = -1.f;
        end = -1.f;
        fatherId = -1;
        match_id = -1;
        type = 0;
        imagNum = 0;
        imgIds.clear();
        dependNum = 0;
        depends.clear();
        functionName = "";
        sfmmem = -1;
        keyMaxImgNum = 2000;
        matchMaxImgNum = 8000;
        sfmId = -1;
        sfm_task_num = -1;
        match_task_num = 0;
        matchIdNum = 0;
        matchIds.clear();
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(int));
        unsigned int msg_len = msg.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&msg_len), sizeof(msg_len));
        byteCrypt.WriteByteDecrypted(out, msg.c_str(), msg_len);
        unsigned int name_len = name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        byteCrypt.WriteByteDecrypted(out, name.c_str(), name_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&begin), sizeof(float));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&end), sizeof(float));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&fatherId), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&match_id), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&type), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&imagNum), sizeof(int));
        for (int i = 0; i < imagNum; i++) {
            int tmp = imgIds[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(int));
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&dependNum), sizeof(int));
        for (int i = 0; i < dependNum; i++) {
            int tmp = depends[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(int));
        }
        unsigned int functionName_len = functionName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&functionName_len), sizeof(functionName_len));
        byteCrypt.WriteByteDecrypted(out, functionName.c_str(), functionName_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&sfmmem), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&keyMaxImgNum), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&matchMaxImgNum), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&sfmId), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&sfm_task_num), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&match_task_num), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&matchIdNum), sizeof(int));
        for (int i = 0; i < matchIdNum; i++) {
            int tmp = matchIds[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(int));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(int));
        unsigned int msg_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&msg_len), sizeof(unsigned int));
        msg.resize(msg_len);
        byteCrypt.ReadByteDecrypted(in, &msg[0], msg_len);
        unsigned int name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&name_len), sizeof(unsigned int));
        name.resize(name_len);
        byteCrypt.ReadByteDecrypted(in, &name[0], name_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&begin), sizeof(float));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&end), sizeof(float));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&fatherId), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&match_id), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&type), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&imagNum), sizeof(int));
        for (int i = 0; i < imagNum; i++) {
            int tmp = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(int));
            imgIds.push_back(tmp);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&dependNum), sizeof(int));
        for (int i = 0; i < dependNum; i++) {
            int tmp = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(int));
            depends.push_back(tmp);
        }
        unsigned int functionName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&functionName_len), sizeof(unsigned int));
        functionName.resize(functionName_len);
        byteCrypt.ReadByteDecrypted(in, &functionName[0], functionName_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&sfmmem), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&keyMaxImgNum), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&matchMaxImgNum), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&sfmId), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&sfm_task_num), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&match_task_num), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&matchIdNum), sizeof(int));
        for (int i = 0; i < matchIdNum; i++) {
            int tmp = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(int));
            matchIds.push_back(tmp);
        }
        return true;
    };
};

struct ROIData {
    int boundary_level1_size;
    int boundary_level2_size;
    std::vector< std::vector< std::vector<double> > > boundary;
    double min_z;
    double max_z;

    ByteCrypt byteCrypt;

    ROIData() {
        boundary_level1_size = 0;
        boundary_level2_size = 0;
        boundary.clear();
        min_z = 0;
        max_z = 0;
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&min_z), sizeof(double));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&max_z), sizeof(double));

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&boundary_level1_size), sizeof(int));
        for (int i = 0; i < boundary_level1_size; ++i) {
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&boundary_level2_size), sizeof(int));
            for (int j = 0; j < boundary_level2_size; ++j) {
                int boundary_level3_size = 2;
                byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&boundary_level3_size), sizeof(int));
                for (int k = 0; k < boundary_level3_size; ++k) {
                    double tmpData = boundary[i][j][k];
                    byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
                }
            }
        }

        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&min_z), sizeof(double));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&max_z), sizeof(double));
        boundary.clear();
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&boundary_level1_size), sizeof(int));
        for (int i = 0; i < boundary_level1_size; ++i) {
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&boundary_level2_size), sizeof(int));
            std::vector< std::vector<double> > thirdLeve2;
            for (int j = 0; j < boundary_level2_size; ++j) {
                int boundary_level3_size = 0;
                byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&boundary_level3_size), sizeof(int));
                std::vector<double> thirdLevel;
                for (int k = 0; k < boundary_level3_size; ++k) {
                    double tmpData;
                    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
                    thirdLevel.push_back(tmpData);
                }
                thirdLeve2.push_back(thirdLevel);
            }
            boundary.push_back(thirdLeve2);
        }
        return true;
    };
};

struct SPTaskInfoFile {
    std::string jobName;
    std::string blockItem;
    std::string projectfile;
    int sdebug;
    bool hasAT;
    std::string ATFile;
    bool hasGCP;
    std::string GCPFile;
    TaskMetaData taskMetaData;
    bool hasATParam;
    ATSettingData atSetting;
    bool hasRecParam;
    ReconstrutionData reconData;
    bool hasGlobalBBox;
    BBoxData globalBBox;
    bool hasROI;
    ROIData roiData;
    std::string ROISrs;

    ByteCrypt byteCrypt;

    SPTaskInfoFile() {
        jobName = "";
        blockItem = "";
        projectfile = "";
        hasAT = false;
        ATFile = "";
        hasGCP = false;
        GCPFile = "";
        sdebug = 1;
        hasATParam = false;
        hasRecParam = false;
        hasGlobalBBox = false;
        hasROI = false;
        ROISrs = "";
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "TASKDEF-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 16); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 16); 

        unsigned int jobName_len = jobName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&jobName_len), sizeof(jobName_len));
        byteCrypt.WriteByteDecrypted(out, jobName.c_str(), jobName_len);
        unsigned int blockItem_len = blockItem.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&blockItem_len), sizeof(blockItem_len));
        byteCrypt.WriteByteDecrypted(out, blockItem.c_str(), blockItem_len);
        unsigned int projectfile_len = projectfile.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&projectfile_len), sizeof(projectfile_len));
        byteCrypt.WriteByteDecrypted(out, projectfile.c_str(), projectfile_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&sdebug), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasAT), sizeof(bool));
        if (hasAT) {
            unsigned int ATFile_len = ATFile.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&ATFile_len), sizeof(ATFile_len));
            byteCrypt.WriteByteDecrypted(out, ATFile.c_str(), ATFile_len);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasGCP), sizeof(bool));
        if (hasGCP) {
            unsigned int GCPFile_len = GCPFile.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&GCPFile_len), sizeof(GCPFile_len));
            byteCrypt.WriteByteDecrypted(out, GCPFile.c_str(), GCPFile_len);
        }
        taskMetaData.Serialize(out);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasATParam), sizeof(bool));
        if (hasATParam) {
            atSetting.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasRecParam), sizeof(bool));
        if (hasRecParam) {
            reconData.Serialize(out);
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasGlobalBBox), sizeof(bool));
            if (hasGlobalBBox) {
                globalBBox.Serialize(out);
            }
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasROI), sizeof(bool));
            if (hasROI) {
                roiData.Serialize(out);
            }
            unsigned int ROISrs_len = ROISrs.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&ROISrs_len), sizeof(ROISrs_len));
            byteCrypt.WriteByteDecrypted(out, ROISrs.c_str(), ROISrs_len);
        }
        
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[16];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 16);
        const char SOURCE_HEADER_LABEL[] = "TASKDEF-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 16)) {
            return false;
        }

        unsigned int jobName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&jobName_len), sizeof(unsigned int));
        jobName.resize(jobName_len);
        byteCrypt.ReadByteDecrypted(in, &jobName[0], jobName_len);
        unsigned int blockItem_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&blockItem_len), sizeof(unsigned int));
        blockItem.resize(blockItem_len);
        byteCrypt.ReadByteDecrypted(in, &blockItem[0], blockItem_len);
        unsigned int projectfile_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&projectfile_len), sizeof(unsigned int));
        projectfile.resize(projectfile_len);
        byteCrypt.ReadByteDecrypted(in, &projectfile[0], projectfile_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&sdebug), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasAT), sizeof(bool));
        if (hasAT) {
            unsigned int ATFile_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&ATFile_len), sizeof(unsigned int));
            ATFile.resize(ATFile_len);
            byteCrypt.ReadByteDecrypted(in, &ATFile[0], ATFile_len);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasGCP), sizeof(bool));
        if (hasGCP) {
            unsigned int GCPFile_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&GCPFile_len), sizeof(unsigned int));
            GCPFile.resize(GCPFile_len);
            byteCrypt.ReadByteDecrypted(in, &GCPFile[0], GCPFile_len);
        }
        taskMetaData.Deserialize(in);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasATParam), sizeof(bool));
        if (hasATParam) {
            atSetting.Deserialize(in);
        }       
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasRecParam), sizeof(bool));
        if (hasRecParam) {
            reconData.Deserialize(in);
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasGlobalBBox), sizeof(bool));
            if (hasGlobalBBox) {
                globalBBox.Deserialize(in);
            }
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasROI), sizeof(bool));
            if (hasROI) {
                roiData.Deserialize(in);
            }
            unsigned int ROISrs_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&ROISrs_len), sizeof(unsigned int));
            ROISrs.resize(ROISrs_len);
            byteCrypt.ReadByteDecrypted(in, &ROISrs[0], ROISrs_len);
        }
       
        return true;
    };
};




struct BlockData {
    std::string blockName;
    std::string blockPath;

    ByteCrypt byteCrypt;

    BlockData() {
        blockName = "";
        blockPath = "";
    }

    bool Serialize(std::ofstream& out) const {
        unsigned int blockName_len = blockName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&blockName_len), sizeof(blockName_len));
        byteCrypt.WriteByteDecrypted(out, blockName.c_str(), blockName_len);
        unsigned int blockPath_len = blockPath.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&blockPath_len), sizeof(blockPath_len));
        byteCrypt.WriteByteDecrypted(out, blockPath.c_str(), blockPath_len);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        unsigned int blockName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&blockName_len), sizeof(unsigned int));
        blockName.resize(blockName_len);
        byteCrypt.ReadByteDecrypted(in, &blockName[0], blockName_len);
        unsigned int blockPath_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&blockPath_len), sizeof(unsigned int));
        blockPath.resize(blockPath_len);
        byteCrypt.ReadByteDecrypted(in, &blockPath[0], blockPath_len);
        return true;
    };
};


struct ProjectFile {
    std::string projectName;
    int blockNum;
    std::vector<BlockData> blocks;

    ByteCrypt byteCrypt;

    ProjectFile() {
        projectName = "";
        blockNum = 0;
        blocks.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "PROJECT-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 16); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 16); 

        unsigned int projectName_len = projectName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&projectName_len), sizeof(projectName_len));
        byteCrypt.WriteByteDecrypted(out, projectName.c_str(), projectName_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&blockNum), sizeof(int));
        for (int i = 0; i < blockNum; i++)
        {
            blocks[i].Serialize(out);
        }
        
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[16];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 16);
        const char SOURCE_HEADER_LABEL[] = "PROJECT-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 16)) {
            return false;
        }

        unsigned int projectName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&projectName_len), sizeof(unsigned int));
        projectName.resize(projectName_len);
        byteCrypt.ReadByteDecrypted(in, &projectName[0], projectName_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&blockNum), sizeof(int));
        blocks.clear();
        for (int i = 0; i < blockNum; ++i)
        {
            BlockData blockData;
            blockData.Deserialize(in);
            blocks.push_back(blockData);
        }
        return true;
    };
};

struct EngineFile {
    std::string version;
    std::string hostName;
    std::string userName;
    std::string projectName;
    std::string startTime;
    std::string endTime;
    std::string ipAddr;
    int status;
    int totalMem;
    int freeMem;
    int processId;
    std::string taskFile;

    ByteCrypt byteCrypt;

    EngineFile() {
        version = "";
        hostName = "";
        userName = "";
        userName = "";
        projectName = "";
        endTime = "";
        ipAddr = "";
        status = -1;
        totalMem = 0;
        freeMem = 0;
        processId = -1;
        taskFile = "";
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "ENGINE-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 15); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 15); 

        unsigned int version_len = version.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&version_len), sizeof(version_len));
        byteCrypt.WriteByteDecrypted(out, version.c_str(), version_len);
        unsigned int hostName_len = hostName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hostName_len), sizeof(hostName_len));
        byteCrypt.WriteByteDecrypted(out, hostName.c_str(), hostName_len);
        unsigned int userName_len = userName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&userName_len), sizeof(userName_len));
        byteCrypt.WriteByteDecrypted(out, userName.c_str(), userName_len);
        unsigned int projectName_len = projectName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&projectName_len), sizeof(projectName_len));
        byteCrypt.WriteByteDecrypted(out, projectName.c_str(), projectName_len);
        unsigned int startTime_len = startTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&startTime_len), sizeof(startTime_len));
        byteCrypt.WriteByteDecrypted(out, startTime.c_str(), startTime_len);
        unsigned int endTime_len = endTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&endTime_len), sizeof(endTime_len));
        byteCrypt.WriteByteDecrypted(out, endTime.c_str(), endTime_len);
        unsigned int ipAddr_len = ipAddr.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&ipAddr_len), sizeof(ipAddr_len));
        byteCrypt.WriteByteDecrypted(out, ipAddr.c_str(), ipAddr_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&status), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&totalMem), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&freeMem), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&processId), sizeof(int));
        unsigned int taskFile_len = taskFile.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&taskFile_len), sizeof(taskFile_len));
        byteCrypt.WriteByteDecrypted(out, taskFile.c_str(), taskFile_len);

        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[15];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 15);
        const char SOURCE_HEADER_LABEL[] = "ENGINE-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 15)) {
            return false;
        }

        unsigned int version_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&version_len), sizeof(unsigned int));
        version.resize(version_len);
        byteCrypt.ReadByteDecrypted(in, &version[0], version_len);
        unsigned int hostName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hostName_len), sizeof(unsigned int));
        hostName.resize(hostName_len);
        byteCrypt.ReadByteDecrypted(in, &hostName[0], hostName_len);
        unsigned int userName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&userName_len), sizeof(unsigned int));
        userName.resize(userName_len);
        byteCrypt.ReadByteDecrypted(in, &userName[0], userName_len);
        unsigned int projectName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&projectName_len), sizeof(unsigned int));
        projectName.resize(projectName_len);
        byteCrypt.ReadByteDecrypted(in, &projectName[0], projectName_len);
        unsigned int startTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&startTime_len), sizeof(unsigned int));
        startTime.resize(startTime_len);
        byteCrypt.ReadByteDecrypted(in, &startTime[0], startTime_len);
        unsigned int endTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&endTime_len), sizeof(unsigned int));
        endTime.resize(endTime_len);
        byteCrypt.ReadByteDecrypted(in, &endTime[0], endTime_len);
        unsigned int ipAddr_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&ipAddr_len), sizeof(unsigned int));
        ipAddr.resize(ipAddr_len);
        byteCrypt.ReadByteDecrypted(in, &ipAddr[0], ipAddr_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&status), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&totalMem), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&freeMem), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&processId), sizeof(int));
        unsigned int taskFile_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&taskFile_len), sizeof(unsigned int));
        taskFile.resize(taskFile_len);
        byteCrypt.ReadByteDecrypted(in, &taskFile[0], taskFile_len);
        return true;
    };
};

struct TrackItemData {
    unsigned int image_id;
    std::array<double, 2> uv;

    ByteCrypt byteCrypt;

    TrackItemData() {
        image_id = -1;
        uv.fill(0.0);
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&image_id), sizeof(unsigned int));
        int uv_size = 2;
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&uv_size), sizeof(int));
        for (int i = 0; i < uv_size; i++) {
            double tmp = uv[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(double));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&image_id), sizeof(unsigned int));
        int uv_size = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&uv_size), sizeof(int));
        for (int i = 0; i < uv_size; i++) {
            double tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(double));
            uv[i] = tmp;
        }
        return true;
    };
};

struct PointItemData {
    unsigned long long index_point3d;
    std::array<double, 3> xyz;
    std::array<int, 3> rgb;
    int num_elements;
    std::vector<TrackItemData> vec_trackele;

    ByteCrypt byteCrypt;

    PointItemData() {
        index_point3d = -1;
        xyz.fill(0.0);
        rgb.fill(0);
        num_elements = 0;
        vec_trackele.clear();
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&index_point3d), sizeof(unsigned long long));
        int xyz_size = 3;
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&xyz_size), sizeof(int));
        for (int i = 0; i < xyz_size; i++) {
            double tmp = xyz[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(double));
        }
        int rgb_size = 3;
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&rgb_size), sizeof(int));
        for (int i = 0; i < rgb_size; i++) {
            int tmp = rgb[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(int));
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_elements), sizeof(int));
        for (int i = 0; i < num_elements; i++) {
            vec_trackele[i].Serialize(out);
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&index_point3d), sizeof(unsigned long long));
        int xyz_size = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&xyz_size), sizeof(int));
        for (int i = 0; i < xyz_size; i++) {
            double tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(double));
            xyz[i] = tmp;
        }
        int rgb_size = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&rgb_size), sizeof(int));
        for (int i = 0; i < rgb_size; i++) {
            int tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(int));
            rgb[i] = tmp;
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_elements), sizeof(int));
        for (int i = 0; i < num_elements; i++) {
            TrackItemData trackItemData;
            trackItemData.Deserialize(in);
            vec_trackele.push_back(trackItemData);
        }
        return true;
    };
};

struct TiePointsFile {
    unsigned long long num_tiepoints;
    std::vector<PointItemData> pointVec;

    ByteCrypt byteCrypt;

    TiePointsFile() {
        num_tiepoints = 0;
        pointVec.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "TIEPOINT-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 17); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 17); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_tiepoints), sizeof(unsigned long long));
        for (int i = 0; i < num_tiepoints; i++)
        {
            pointVec[i].Serialize(out);
        }

        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[17];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 17);
        const char SOURCE_HEADER_LABEL[] = "TIEPOINT-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 17)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_tiepoints), sizeof(unsigned long long));
        pointVec.clear();
        for (int i = 0; i < num_tiepoints; ++i)
        {
            PointItemData pointItemData;
            pointItemData.Deserialize(in);
            pointVec.push_back(pointItemData);
        }
        return true;
    };
};

struct ReViewsBinFile {
    int num;
    std::vector<int> image_ids_;

    ByteCrypt byteCrypt;

    ReViewsBinFile() {
        num = 0;
        image_ids_.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "REVIEWS-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 16); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 16); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num), sizeof(int));
        for (int i = 0; i < num; i++)
        {
            int tmp = image_ids_[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(int));;
        }

        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[16];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 16);
        const char SOURCE_HEADER_LABEL[] = "REVIEWS-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 16)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num), sizeof(int));
        image_ids_.clear();
        for (int i = 0; i < num; ++i)
        {
            int tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(int));
            image_ids_.push_back(tmp);
        }
        return true;
    };
};

struct CBItemData {
    unsigned int imageid;
    std::array<double, 3> colors;

    ByteCrypt byteCrypt;

    CBItemData() {
        imageid = -1;
        colors.fill(0.0);
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&imageid), sizeof(unsigned int));
        for (int i = 0; i < 3; i++)
        {
            double tmp = colors[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(double));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&imageid), sizeof(unsigned int));
        for (int i = 0; i < 3; ++i)
        {
            double tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(double));
            colors[i] = tmp;
        }
        return true;
    };
};

struct CBBinFile {
    int num;
    
    std::vector<CBItemData> colorsParams;

    ByteCrypt byteCrypt;

    CBBinFile() {
        num = 0;
        colorsParams.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "CB-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 11); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 11); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num), sizeof(int));
        for (int i = 0; i < num; i++)
        {
            CBItemData cbItemData = colorsParams[i];
            cbItemData.Serialize(out);
        }

        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[11];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 11);
        const char SOURCE_HEADER_LABEL[] = "CB-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 11)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num), sizeof(int));
        colorsParams.clear();
        for (int i = 0; i < num; ++i)
        {
            CBItemData cbItemData;
            cbItemData.Deserialize(in);
            colorsParams.push_back(cbItemData);
        }
        return true;
    };
};

struct GCPItem {
    unsigned int imageid;
    std::array<double, 2> xy;

    ByteCrypt byteCrypt;

    GCPItem() {
        imageid = -1;
        xy.fill(0.0);
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&imageid), sizeof(unsigned int));
        for (int i = 0; i < 2; i++)
        {
            double tmp = xy[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(double));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&imageid), sizeof(unsigned int));
        for (int i = 0; i < 2; ++i)
        {
            double tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(double));
            xy[i] = tmp;
        }
        return true;
    };
};

struct GCPData {    
    unsigned int pointid;
    std::array<double, 3> cp_pos;
    std::array<double, 2> weight;
    std::string name;
    int category;
    int num_eles;
    std::vector<GCPItem> elesVec;
    bool hasExtraParam;
    unsigned int srsid;

    ByteCrypt byteCrypt;

    GCPData() {       
        pointid = -1;
        cp_pos.fill(0.0);
        weight.fill(0.0);
        name = "";
        category = -1;
        num_eles = 0;
        elesVec.clear();
        hasExtraParam = false;
        srsid = -1;
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&pointid), sizeof(unsigned int));
        for (int i = 0; i < 3; i++)
        {
            double tmp = cp_pos[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(double));
        }
        for (int i = 0; i < 2; i++)
        {
            double tmp = weight[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(double));
        }
        unsigned int name_len = name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        byteCrypt.WriteByteDecrypted(out, name.c_str(), name_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&category), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_eles), sizeof(int));
        for (int i = 0; i < num_eles; i++)
        {
            GCPItem gcpItem = elesVec[i];
            gcpItem.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasExtraParam), sizeof(bool));
        if (hasExtraParam) {
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&srsid), sizeof(unsigned int));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&pointid), sizeof(unsigned int));
        for (int i = 0; i < 3; ++i)
        {
            double tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(double));
            cp_pos[i] = tmp;
        }
        for (int i = 0; i < 2; ++i)
        {
            double tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(double));
            weight[i] = tmp;
        }
        unsigned int name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&name_len), sizeof(unsigned int));
        name.resize(name_len);
        byteCrypt.ReadByteDecrypted(in, &name[0], name_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&category), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_eles), sizeof(int));
        elesVec.clear();
        for (int i = 0; i < num_eles; ++i)
        {
            GCPItem gcpItem;
            gcpItem.Deserialize(in);
            elesVec.push_back(gcpItem);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasExtraParam), sizeof(bool));
        if (hasExtraParam) {
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&srsid), sizeof(unsigned int));
        }
        return true;
    };
};

struct UsedPointData {
    unsigned int imageid;
    std::array<double, 2> xy;

    ByteCrypt byteCrypt;

    UsedPointData() {
        imageid = -1;
        xy.fill(0.0);
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&imageid), sizeof(unsigned int));
        for (int i = 0; i < 2; i++)
        {
            double tmp = xy[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(double));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&imageid), sizeof(unsigned int));
        for (int i = 0; i < 2; ++i)
        {
            double tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(double));
            xy[i] = tmp;
        }
        return true;
    };
};

struct ATCameraData {
    CameraData cameraData;
    int num_images;
    std::vector<ImageData> images;

    ByteCrypt byteCrypt;

    ATCameraData() {
        num_images = 0;
        images.clear();
    }

    bool Serialize(std::ofstream& out) const {
        cameraData.Serialize(out);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_images), sizeof(int));
        for (int i = 0; i < num_images; i++)
        {
            ImageData imageData = images[i];
            imageData.Serialize(out);
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        cameraData.Deserialize(in);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_images), sizeof(int));
        images.clear();
        for (int i = 0; i < num_images; ++i)
        {
            ImageData imageData;
            imageData.Deserialize(in);
            images.push_back(imageData);
        }
        return true;
    };
};

struct ATBinFile {
    int version;
    std::string definition;
    int num_photogroups;
    std::vector<ATCameraData> photoGroups;
    int num_tiepoints;
    std::vector<PointItemData> pointVec;
    int num_controlpoints;
    std::string gcpDefine;
    std::vector<GCPData> gcpVec;
    int num_userpoints;
    std::vector< std::vector<UsedPointData> > usedPointVec;

    ByteCrypt byteCrypt;

    ATBinFile() {
        version = 0;
        definition = "";
        num_photogroups = 0;
        photoGroups.clear();
        num_tiepoints = 0;
        pointVec.clear();
        num_controlpoints = 0;
        gcpDefine = "";
        gcpVec.clear();
        num_userpoints = 0;
        usedPointVec.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "AT-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 11); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 11); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&version), sizeof(int));
        unsigned int def_len = definition.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&def_len), sizeof(def_len));
        byteCrypt.WriteByteDecrypted(out, definition.c_str(), def_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_photogroups), sizeof(int));
        for (int i = 0; i < num_photogroups; i++)
        {
            ATCameraData atCameraData = photoGroups[i];
            atCameraData.Serialize(out);
        }      
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_tiepoints), sizeof(int));
        for (int i = 0; i < num_tiepoints; i++)
        {
            PointItemData pointItemData = pointVec[i];
            pointItemData.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_controlpoints), sizeof(int));
        if (num_controlpoints) {
            unsigned int def_len = gcpDefine.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&def_len), sizeof(def_len));
            byteCrypt.WriteByteDecrypted(out, gcpDefine.c_str(), def_len);
        }
        for (int i = 0; i < num_controlpoints; i++)
        {
            GCPData pcgData = gcpVec[i];
            pcgData.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_userpoints), sizeof(int));
        for (int i = 0; i < num_userpoints; i++)
        {
            int num_eles = usedPointVec[i].size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_eles), sizeof(int));
            for (int j = 0; j < num_eles; j++)
            {
                UsedPointData usedPointData = usedPointVec[i][j];
                usedPointData.Serialize(out);
            }
            
        }

        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[11];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 11);
        const char SOURCE_HEADER_LABEL[] = "AT-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 11)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&version), sizeof(int));
        unsigned int definition_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&definition_len), sizeof(unsigned int));
        definition.resize(definition_len);
        byteCrypt.ReadByteDecrypted(in, &definition[0], definition_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_photogroups), sizeof(int));
        photoGroups.clear();
        for (int i = 0; i < num_photogroups; ++i)
        {
            ATCameraData atCameraData;
            atCameraData.Deserialize(in);
            photoGroups.push_back(atCameraData);
        }      
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_tiepoints), sizeof(int));
        pointVec.clear();
        for (int i = 0; i < num_tiepoints; ++i)
        {
            PointItemData pointItemData;
            pointItemData.Deserialize(in);
            pointVec.push_back(pointItemData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_controlpoints), sizeof(int));
        gcpVec.clear();
        if (num_controlpoints) {
            unsigned int gcpDefine_len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gcpDefine_len), sizeof(unsigned int));
            gcpDefine.resize(gcpDefine_len);
            byteCrypt.ReadByteDecrypted(in, &gcpDefine[0], gcpDefine_len);
        }
        for (int i = 0; i < num_controlpoints; ++i)
        {
            GCPData gcpData;
            gcpData.Deserialize(in);
            gcpVec.push_back(gcpData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_userpoints), sizeof(int));
        usedPointVec.clear();
        for (int i = 0; i < num_userpoints; ++i)
        {
            int num_eles = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_eles), sizeof(int));
            std::vector<UsedPointData> level2;
            for (int j = 0; j < num_eles; j++)
            {               
                UsedPointData usedPointData;
                usedPointData.Deserialize(in);
                level2.push_back(usedPointData);
            }
            usedPointVec.push_back(level2);
        }
        return true;
    };
};

struct SRSItemData {
    int id;
    std::string name;
    int type;
    std::string definition;

    ByteCrypt byteCrypt;

    SRSItemData() {
        id = 0;
        name = "";
        type = 0;
        definition = "";
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(int));
        unsigned int name_len = name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        byteCrypt.WriteByteDecrypted(out, name.c_str(), name_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&type), sizeof(int));
        unsigned int def_len = definition.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&def_len), sizeof(def_len));
        byteCrypt.WriteByteDecrypted(out, definition.c_str(), def_len);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(int));
        unsigned int name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&name_len), sizeof(unsigned int));
        name.resize(name_len);
        byteCrypt.ReadByteDecrypted(in, &name[0], name_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&type), sizeof(int));
        unsigned int definition_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&definition_len), sizeof(unsigned int));
        definition.resize(definition_len);
        byteCrypt.ReadByteDecrypted(in, &definition[0], definition_len);
        return true;
    };
};

struct UserPointVecData {
    unsigned int id;
    std::string name;
    unsigned int num_ele;
    std::vector<UsedPointData> usedPointVec;
    bool hasguideimage;
    unsigned int image_id_forguide;

    ByteCrypt byteCrypt;

    UserPointVecData() {
        id = -1;
        name = "";
        num_ele = 0;
        usedPointVec.clear();
        hasguideimage = false;
        image_id_forguide = 0;
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(unsigned int));
        unsigned int name_len = name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        byteCrypt.WriteByteDecrypted(out, name.c_str(), name_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_ele), sizeof(unsigned int));
        for (int i = 0; i < num_ele; i++)
        {
            UsedPointData usedPointData = usedPointVec[i];
            usedPointData.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&hasguideimage), sizeof(bool));
        if (hasguideimage) {
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&image_id_forguide), sizeof(unsigned int));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(unsigned int));
        unsigned int name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&name_len), sizeof(unsigned int));
        name.resize(name_len);
        byteCrypt.ReadByteDecrypted(in, &name[0], name_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_ele), sizeof(unsigned int));
        usedPointVec.clear();
        for (int i = 0; i < num_ele; ++i)
        {
            UsedPointData usedPointData;
            usedPointData.Deserialize(in);
            usedPointVec.push_back(usedPointData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&hasguideimage), sizeof(bool));
        if (hasguideimage) {
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&image_id_forguide), sizeof(unsigned int));
        }
        return true;
    };
};

struct ATItemData {
    int data_idx;
    int num_srs;
    std::vector<SRSItemData> srsVec;
    std::string block_name;
    std::string block_description;
    unsigned int blockSRS_id;
    std::string local_srs_definition;
    int num_photogroups;
    std::vector<ATCameraData> photoGroups;
    int num_controlpoints;
    std::vector<GCPData> gcpVec;
    int num_userpoints;
    std::vector<UserPointVecData> usedPointVec;

    ByteCrypt byteCrypt;

    ATItemData() {
        data_idx = 0;
        num_srs = 0;
        srsVec.clear();
        block_name = "";
        block_description = "";
        blockSRS_id = -1;
        local_srs_definition = "";
        num_photogroups = 0;
        photoGroups.clear();
        num_controlpoints = 0;
        gcpVec.clear();
        num_userpoints = 0;
        usedPointVec.clear();
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&data_idx), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_srs), sizeof(int));
        for (int i = 0; i < num_srs; i++)
        {
            SRSItemData srsItemData = srsVec[i];
            srsItemData.Serialize(out);
        }
        unsigned int block_name_len = block_name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&block_name_len), sizeof(unsigned int));
        byteCrypt.WriteByteDecrypted(out, block_name.c_str(), block_name_len);
        unsigned int block_description_len = block_description.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&block_description_len), sizeof(unsigned int));
        byteCrypt.WriteByteDecrypted(out, block_description.c_str(), block_description_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&blockSRS_id), sizeof(unsigned int));
        unsigned int local_srs_definition_len = local_srs_definition.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&local_srs_definition_len), sizeof(unsigned int));
        byteCrypt.WriteByteDecrypted(out, local_srs_definition.c_str(), local_srs_definition_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_photogroups), sizeof(int));
        for (int i = 0; i < num_photogroups; i++)
        {
            ATCameraData atCameraData = photoGroups[i];
            atCameraData.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_controlpoints), sizeof(int));
        for (int i = 0; i < num_controlpoints; i++)
        {
            GCPData pcgData = gcpVec[i];
            pcgData.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&num_userpoints), sizeof(int));
        for (int i = 0; i < num_userpoints; i++)
        {
            UserPointVecData userPointVecData = usedPointVec[i];
            userPointVecData.Serialize(out);
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&data_idx), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_srs), sizeof(int));
        srsVec.clear();
        for (int i = 0; i < num_srs; ++i)
        {
            SRSItemData srsItemData;
            srsItemData.Deserialize(in);
            srsVec.push_back(srsItemData);
        }
        unsigned int block_name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&block_name_len), sizeof(unsigned int));
        block_name.resize(block_name_len);
        byteCrypt.ReadByteDecrypted(in, &block_name[0], block_name_len);
        unsigned int block_description_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&block_description_len), sizeof(unsigned int));
        block_description.resize(block_description_len);
        byteCrypt.ReadByteDecrypted(in, &block_description[0], block_description_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&blockSRS_id), sizeof(unsigned int));
        unsigned int local_srs_definition_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&local_srs_definition_len), sizeof(unsigned int));
        local_srs_definition.resize(local_srs_definition_len);
        byteCrypt.ReadByteDecrypted(in, &local_srs_definition[0], local_srs_definition_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_photogroups), sizeof(int));
        photoGroups.clear();
        for (int i = 0; i < num_photogroups; ++i)
        {
            ATCameraData atCameraData;
            atCameraData.Deserialize(in);
            photoGroups.push_back(atCameraData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_controlpoints), sizeof(int));
        gcpVec.clear();
        for (int i = 0; i < num_controlpoints; ++i)
        {
            GCPData gcpData;
            gcpData.Deserialize(in);
            gcpVec.push_back(gcpData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&num_userpoints), sizeof(int));
        usedPointVec.clear();
        for (int i = 0; i < num_userpoints; ++i)
        {
            UserPointVecData userPointVecData;
            userPointVecData.Deserialize(in);
            usedPointVec.push_back(userPointVecData);
        }
        return true;
    };
};

struct ATBlockBinFile {
    int version;
    int atdatasize;
    std::vector<ATItemData> atList;

    ByteCrypt byteCrypt;

    ATBlockBinFile() {
        version = 0;
        atdatasize = 0;
        atList.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "ATBLOCK-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 16); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 16); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&version), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&atdatasize), sizeof(int));
        for (int i = 0; i < atdatasize; i++)
        {
            ATItemData atItemData = atList[i];
            atItemData.Serialize(out);
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[16];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 16);
        const char SOURCE_HEADER_LABEL[] = "ATBLOCK-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 16)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&version), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&atdatasize), sizeof(int));
        atList.clear();
        for (int i = 0; i < atdatasize; ++i)
        {
            ATItemData atItemData;
            atItemData.Deserialize(in);
            atList.push_back(atItemData);
        }
       
        return true;
    };
};

struct SRSFile {
    int type;
    std::string definition;
    std::array<double, 3> ori;
    std::string espgCode;

    ByteCrypt byteCrypt;

    SRSFile() {
        type = 1;
        definition = "";
        ori.fill(0.0);
        espgCode = "";
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "SRS-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 12); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 12); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&type), sizeof(int));
        unsigned int definition_len = definition.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&definition_len), sizeof(definition_len));
        byteCrypt.WriteByteDecrypted(out, definition.c_str(), definition_len);
        for (int i = 0; i < 3; ++i) {
            double tmpData = ori[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
        }
        unsigned int espg_len = espgCode.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&espg_len), sizeof(espg_len));
        byteCrypt.WriteByteDecrypted(out, espgCode.c_str(), espg_len);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[12];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 12);
        const char SOURCE_HEADER_LABEL[] = "SRS-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 12)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&type), sizeof(int));
        unsigned int definition_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&definition_len), sizeof(unsigned int));
        definition.resize(definition_len);
        byteCrypt.ReadByteDecrypted(in, &definition[0], definition_len);
        for (int i = 0; i < 3; ++i) {
            double tmpData;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
            ori[i] = (tmpData);
        }
        unsigned int espg_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&espg_len), sizeof(unsigned int));
        espgCode.resize(espg_len);
        byteCrypt.ReadByteDecrypted(in, &espgCode[0], espg_len);
        return true;
    };
};

struct POSItem {
    int id;
    std::array<double, 3> accuracy;
    std::array<double, 3> position;

    ByteCrypt byteCrypt;

    POSItem() {
        id = -1;
        accuracy.fill(0.0);
        position.fill(0.0);
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(int));
        for (int i = 0; i < 3; ++i) {
            double tmpData = accuracy[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
        }
        for (int i = 0; i < 3; ++i) {
            double tmpData = position[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(int));
        for (int i = 0; i < 3; ++i) {
            double tmpData;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
            accuracy[i] = (tmpData);
        }
        for (int i = 0; i < 3; ++i) {
            double tmpData;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
            position[i] = (tmpData);
        }
        return true;
    };
};

struct POSFile {
    int type;
    std::string definition;
    std::array<double, 3> ori;
    std::string espgCode;
    int posNum;
    std::vector<POSItem> posList;
    int fixNum;
    std::vector<int> fixIdList;

    ByteCrypt byteCrypt;

    POSFile() {
        type = 1;
        definition = "";
        ori.fill(0.0);
        espgCode = "";
        posNum = 0;
        posList.clear();
        fixNum = 0;
        fixIdList.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "POS-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 12); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 12); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&type), sizeof(int));
        unsigned int definition_len = definition.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&definition_len), sizeof(definition_len));
        byteCrypt.WriteByteDecrypted(out, definition.c_str(), definition_len);
        for (int i = 0; i < 3; ++i) {
            double tmpData = ori[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
        }
        unsigned int espg_len = espgCode.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&espg_len), sizeof(espg_len));
        byteCrypt.WriteByteDecrypted(out, espgCode.c_str(), espg_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&posNum), sizeof(int));
        for (int i = 0; i < posNum; ++i) {
            POSItem posItem = posList[i];
            posItem.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&fixNum), sizeof(int));
        for (int i = 0; i < fixNum; ++i) {
            int tmp = fixIdList[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmp), sizeof(int));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[12];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 12);
        const char SOURCE_HEADER_LABEL[] = "POS-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 12)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&type), sizeof(int));
        unsigned int definition_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&definition_len), sizeof(unsigned int));
        definition.resize(definition_len);
        byteCrypt.ReadByteDecrypted(in, &definition[0], definition_len);
        for (int i = 0; i < 3; ++i) {
            double tmpData;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
            ori[i] = (tmpData);
        }
        unsigned int espg_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&espg_len), sizeof(unsigned int));
        espgCode.resize(espg_len);
        byteCrypt.ReadByteDecrypted(in, &espgCode[0], espg_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&posNum), sizeof(int));
        for (int i = 0; i < posNum; ++i) {
            POSItem posItem;
            posItem.Deserialize(in);
            posList.push_back(posItem);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&fixNum), sizeof(int));
        for (int i = 0; i < fixNum; ++i) {
            int tmp;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmp), sizeof(int));
            fixIdList.push_back(tmp);
        }
        return true;
    };
};

struct PIDFile {
    long long int pid;
    std::string lastActivateTime;
    std::string taskFile;

    ByteCrypt byteCrypt;

    PIDFile() {
        pid = -1;
        lastActivateTime = "";
        taskFile = "";
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "PID-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 12); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 12); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&pid), sizeof(long long int));
        unsigned int lastActivateTime_len = lastActivateTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&lastActivateTime_len), sizeof(lastActivateTime_len));
        byteCrypt.WriteByteDecrypted(out, lastActivateTime.c_str(), lastActivateTime_len);
        unsigned int taskFile_len = taskFile.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&taskFile_len), sizeof(taskFile_len));
        byteCrypt.WriteByteDecrypted(out, taskFile.c_str(), taskFile_len);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[12];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 12);
        const char SOURCE_HEADER_LABEL[] = "PID-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 12)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&pid), sizeof(long long int));
        unsigned int lastActivateTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&lastActivateTime_len), sizeof(unsigned int));
        lastActivateTime.resize(lastActivateTime_len);
        byteCrypt.ReadByteDecrypted(in, &lastActivateTime[0], lastActivateTime_len);
        unsigned int taskFile_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&taskFile_len), sizeof(unsigned int));
        taskFile.resize(taskFile_len);
        byteCrypt.ReadByteDecrypted(in, &taskFile[0], taskFile_len);
        return true;
    };
};

struct StaticMapData {
    std::string key;
    int value;

    ByteCrypt byteCrypt;

    StaticMapData() {
        key = "";
        value = 0;
    }

    bool Serialize(std::ofstream& out) const {        
        unsigned int key_len = key.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        byteCrypt.WriteByteDecrypted(out, key.c_str(), key_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&value), sizeof(int));
        return true;
    };

    bool Deserialize(std::ifstream& in) {        
        unsigned int key_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&key_len), sizeof(unsigned int));
        key.resize(key_len);
        byteCrypt.ReadByteDecrypted(in, &key[0], key_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&value), sizeof(int));
        return true;
    };
};

struct EngineInfoData {
    std::string versionName;
    std::string versionCode;
    std::string startTime;
    std::string quitTime;
    int photoMapNum;
    std::vector<StaticMapData> photoMap;
    int percentMapNum;
    std::vector<StaticMapData> percentMap;

    ByteCrypt byteCrypt;

    EngineInfoData() {
        versionName = "";
        versionCode = "";
        startTime = "";
        quitTime = "";
        photoMapNum = 0;
        photoMap.clear();
        percentMapNum = 0;
        percentMap.clear();
    }

    bool Serialize(std::ofstream& out) const {
        unsigned int versionName_len = versionName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&versionName_len), sizeof(versionName_len));
        byteCrypt.WriteByteDecrypted(out, versionName.c_str(), versionName_len);
        unsigned int versionCode_len = versionCode.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&versionCode_len), sizeof(versionCode_len));
        byteCrypt.WriteByteDecrypted(out, versionCode.c_str(), versionCode_len);
        unsigned int startTime_len = startTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&startTime_len), sizeof(startTime_len));
        byteCrypt.WriteByteDecrypted(out, startTime.c_str(), startTime_len);
        unsigned int quitTime_len = quitTime.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&quitTime_len), sizeof(quitTime_len));
        byteCrypt.WriteByteDecrypted(out, quitTime.c_str(), quitTime_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&photoMapNum), sizeof(int));
        for (int i = 0; i < photoMapNum; ++i) {
            StaticMapData staticMapData = photoMap[i];
            staticMapData.Serialize(out);
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&percentMapNum), sizeof(int));
        for (int i = 0; i < percentMapNum; ++i) {
            StaticMapData staticMapData = percentMap[i];
            staticMapData.Serialize(out);
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        unsigned int versionName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&versionName_len), sizeof(unsigned int));
        versionName.resize(versionName_len);
        byteCrypt.ReadByteDecrypted(in, &versionName[0], versionName_len);
        unsigned int versionCode_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&versionCode_len), sizeof(unsigned int));
        versionCode.resize(versionCode_len);
        byteCrypt.ReadByteDecrypted(in, &versionCode[0], versionCode_len);
        unsigned int startTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&startTime_len), sizeof(unsigned int));
        startTime.resize(startTime_len);
        byteCrypt.ReadByteDecrypted(in, &startTime[0], startTime_len);
        unsigned int quitTime_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&quitTime_len), sizeof(unsigned int));
        quitTime.resize(quitTime_len);
        byteCrypt.ReadByteDecrypted(in, &quitTime[0], quitTime_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&photoMapNum), sizeof(int));
        for (int i = 0; i < photoMapNum; ++i) {
            StaticMapData staticMapData;
            staticMapData.Deserialize(in);
            photoMap.push_back(staticMapData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&percentMapNum), sizeof(int));
        for (int i = 0; i < percentMapNum; ++i) {
            StaticMapData staticMapData;
            staticMapData.Deserialize(in);
            percentMap.push_back(staticMapData);
        }
        return true;
    };
};

struct EngineStatFile {
    std::string machineCode;
    int versionListNum;
    std::vector<EngineInfoData> versionList;

    ByteCrypt byteCrypt;

    EngineStatFile() {
        machineCode = "";
        versionListNum = 0;
        versionList.clear();
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "ENGINE-STAT-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 20); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 20); 

        unsigned int machineCode_len = machineCode.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&machineCode_len), sizeof(machineCode_len));
        byteCrypt.WriteByteDecrypted(out, machineCode.c_str(), machineCode_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&versionListNum), sizeof(int));
        for (int i = 0; i < versionListNum; ++i) {
            EngineInfoData engineInfoData = versionList[i];
            engineInfoData.Serialize(out);
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[20];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 20);
        const char SOURCE_HEADER_LABEL[] = "ENGINE-STAT-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 20)) {
            return false;
        }

        unsigned int machineCode_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&machineCode_len), sizeof(unsigned int));
        machineCode.resize(machineCode_len);
        byteCrypt.ReadByteDecrypted(in, &machineCode[0], machineCode_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&versionListNum), sizeof(int));
        for (int i = 0; i < versionListNum; ++i) {
            EngineInfoData engineInfoData;
            engineInfoData.Deserialize(in);
            versionList.push_back(engineInfoData);
        }
        return true;
    };
};

struct AuthUseFile {
    int authPhotoNum;
    int usePhotoNum;

    ByteCrypt byteCrypt;

    AuthUseFile() {
        usePhotoNum = 0;
        authPhotoNum = 0;
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "AUTH-USE-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 17); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 17); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&usePhotoNum), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&authPhotoNum), sizeof(int));

        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[17];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 17);
        const char SOURCE_HEADER_LABEL[] = "AUTH-USE-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 17)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&usePhotoNum), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&authPhotoNum), sizeof(int));
        return true;
    };
};

struct ConstraintData {
    unsigned long long id;
    std::string name;
    int type;
    int pointNum;
    std::vector<unsigned long long> pointIds;
    int valueNum;
    std::vector<double> values;

    ByteCrypt byteCrypt;

    ConstraintData() {
        name = "";
        id = 0;
        type = 0;
        pointNum = 0;
        valueNum = 0;
        pointIds.clear();
        values.clear();
    }

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&id), sizeof(unsigned long long));
        unsigned int name_len = name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        byteCrypt.WriteByteDecrypted(out, name.c_str(), name_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&type), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&pointNum), sizeof(int));
        for (int i = 0; i < pointNum; ++i) {
            unsigned long long tmpData = pointIds[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(unsigned long long));
        }
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&valueNum), sizeof(int));
        for (int i = 0; i < valueNum; ++i) {
            double tmpData = values[i];
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&tmpData), sizeof(double));
        }
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&id), sizeof(unsigned long long));
        unsigned int name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&name_len), sizeof(unsigned int));
        name.resize(name_len);
        byteCrypt.ReadByteDecrypted(in, &name[0], name_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&type), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&pointNum), sizeof(int));
        for (int i = 0; i < pointNum; ++i) {
            unsigned long long tmpData = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(unsigned long long));
            pointIds.push_back(tmpData);
        }
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&valueNum), sizeof(int));
        for (int i = 0; i < valueNum; ++i) {
            double tmpData = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&tmpData), sizeof(double));
            values.push_back(tmpData);
        }
        return true;
    };
};

struct ConstraintFile {
    int constraintNum;
    std::vector<ConstraintData> constraintsVec;
    

    ByteCrypt byteCrypt;

    ConstraintFile() {
        constraintNum = 0;
        constraintsVec.clear();
        
    }

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "CONSTRAINT-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 19); 
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 19); 

        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&constraintNum), sizeof(int));
        for (int i = 0; i < constraintNum; ++i) {
            ConstraintData constraintData = constraintsVec[i];
            constraintData.Serialize(out);
        }

        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[19];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 19);
        const char SOURCE_HEADER_LABEL[] = "CONSTRAINT-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 19)) {
            return false;
        }

        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&constraintNum), sizeof(int));
        for (int i = 0; i < constraintNum; ++i) {
            ConstraintData constraintData;
            constraintData.Deserialize(in);
            constraintsVec.push_back(constraintData);
        }
        return true;
    };
};


// ============================================================================
// GenJobFile / GenJobInfoData / GenJobTaskData
//   对标 JobListFile / JobInfoData / RunInfoData+TaskItemData
// ============================================================================

struct GenJobInfoData {
    std::string project_path;   // 项目路径
    std::string block_item;     // Block 名称
    ByteCrypt byteCrypt;

    GenJobInfoData() {
        project_path = "";
        block_item = "";
    }

    bool Serialize(std::ofstream& out) const {
        unsigned int project_path_len = project_path.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&project_path_len),
                                     sizeof(project_path_len));
        byteCrypt.WriteByteDecrypted(out, project_path.c_str(), project_path_len);

        unsigned int block_item_len = block_item.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&block_item_len),
                                     sizeof(block_item_len));
        byteCrypt.WriteByteDecrypted(out, block_item.c_str(), block_item_len);
        return true;
    }

    bool Deserialize(std::ifstream& in) {
        unsigned int project_path_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&project_path_len),
                                    sizeof(unsigned int));
        project_path.resize(project_path_len);
        byteCrypt.ReadByteDecrypted(in, &project_path[0], project_path_len);

        unsigned int block_item_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&block_item_len),
                                    sizeof(unsigned int));
        block_item.resize(block_item_len);
        byteCrypt.ReadByteDecrypted(in, &block_item[0], block_item_len);
        return true;
    }
};

// ---- 指针节 (对标 JobInfoData) ----

// GenJobInfoData — 同上, 仅 project_path + block_item

// ---- 任务数据节 (对标 RunInfoData + TaskItemData 的合集) ----

struct GenJobTaskData {
    // GenJobInfo_s 字段
    int generation_id;
    std::string task_uuid;
    std::string engine_id;
    std::string user_account;
    std::string params_json;
    int status;
    std::string freeze_no;
    std::string result_url;
    std::string preview_url;
    std::string result_path;
    std::string preview_path;
    std::string result_dir;
    std::string error_message;
    int query_retry_count;
    int progress;

    // PointInfoBase 字段 (freeze_no 已在上方 GenJobInfo_s 字段中, 不重复)
    int frozen_points;
    int consumed;
    int refunded;
    int total_balance;
    int available_points;
    bool points_settled;

    ByteCrypt byteCrypt;

    GenJobTaskData() {
        generation_id = 0;
        task_uuid = ""; engine_id = ""; user_account = "";
        params_json = ""; status = 0;
        freeze_no = ""; result_url = ""; preview_url = "";
        result_path = ""; preview_path = ""; result_dir = ""; error_message = "";
        query_retry_count = 0; progress = 0;
        freeze_no = ""; frozen_points = 0; consumed = 0; refunded = 0;
        total_balance = 0; available_points = 0; points_settled = false;
    }

    bool Serialize(std::ofstream& out) const {
        auto ws = [&](const std::string& s) {
            unsigned int len = s.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&len), sizeof(len));
            byteCrypt.WriteByteDecrypted(out, s.c_str(), len);
        };
        auto wi = [&](int v) {
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&v), sizeof(int));
        };

        wi(generation_id);
        ws(task_uuid); ws(engine_id); ws(user_account);
        ws(params_json); wi(status);
        ws(freeze_no); ws(result_url); ws(preview_url);
        ws(result_path); ws(preview_path); ws(result_dir); ws(error_message);
        wi(query_retry_count); wi(progress);

        wi(frozen_points); wi(consumed); wi(refunded);
        wi(total_balance); wi(available_points); wi(points_settled ? 1 : 0);

        return true;
    }

    bool Deserialize(std::ifstream& in) {
        auto rs = [&](std::string& s) {
            unsigned int len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&len), sizeof(unsigned int));
            s.resize(len);
            byteCrypt.ReadByteDecrypted(in, &s[0], len);
        };
        auto ri = [&](int& v) {
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&v), sizeof(int));
        };

        ri(generation_id);
        rs(task_uuid); rs(engine_id); rs(user_account);
        rs(params_json); ri(status);
        rs(freeze_no); rs(result_url); rs(preview_url);
        rs(result_path); rs(preview_path); rs(result_dir); rs(error_message);
        ri(query_retry_count); ri(progress);

        ri(frozen_points); ri(consumed); ri(refunded);
        ri(total_balance); ri(available_points);
        int settledInt = 0; ri(settledInt); points_settled = (settledInt != 0);

        return true;
    }
};

// ---- 顶层容器 (对标 JobListFile) ----

struct GenJobFile {
    std::string jobName;            // 对标 JobListFile::jobName
    GenJobInfoData genJobInfoData;  // 精简指针节
    GenJobTaskData genJobTaskData;  // 任务数据节
    FeedBackData feedBackData;      // 反馈节

    ByteCrypt byteCrypt;
    GenJobFile() {}

    bool Serialize(std::ofstream& out) const {
        const char HEADER[] = "GENJOB-FILE-3MO";
        byteCrypt.WriteByteDecrypted(out, HEADER, 15);

        unsigned int len = jobName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&len), sizeof(len));
        byteCrypt.WriteByteDecrypted(out, jobName.c_str(), len);

        genJobInfoData.Serialize(out);   // 1. 指针节
        genJobTaskData.Serialize(out);   // 2. 任务数据节 (含积分)
        feedBackData.Serialize(out);     // 3. 反馈节
        return true;
    }

    bool Deserialize(std::ifstream& in) {
        char header[15];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        if (std::string(header, 15) != "GENJOB-FILE-3MO") return false;

        unsigned int len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&len), sizeof(unsigned int));
        jobName.resize(len);
        byteCrypt.ReadByteDecrypted(in, &jobName[0], len);

        genJobInfoData.Deserialize(in);
        genJobTaskData.Deserialize(in);
        feedBackData.Deserialize(in);
        return true;
    }
};