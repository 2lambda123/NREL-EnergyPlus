//
// Created by jackcook on 5/6/21.
//

#ifndef CPGFUNCTION_COORDINATES_H
#define CPGFUNCTION_COORDINATES_H

#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace gt::coordinates {

    std::vector<std::tuple<double, double>> configuration(const std::string& shape, int Nx,
                                                int Ny, double Bx, double By);
//    std::vector<std::tuple<double, double>> configuration(const string& shape,
//                                                const string& input_path);

    std::vector<std::tuple<double, double>> rectangle(int Nx, int Ny, double Bx,
                                            double By);
    std::vector<std::tuple<double, double>> Open_rectangle(int Nx, int Ny, double Bx,
                                                 double By);
    std::vector<std::tuple<double, double>> U_shape(int Nx, int Ny, double Bx, double By);
    std::vector<std::tuple<double, double>> L_shape(int Nx, int Ny, double Bx, double By);

//    std::vector<std::tuple<double, double>> import_coordinates_from_file(const string& input_path);
//    void export_coordinates_to_file(const std::vector<std::tuple<double, double>> &coordinates,
//                                    const string& output_path);

}

#endif //CPGFUNCTION_COORDINATES_H
