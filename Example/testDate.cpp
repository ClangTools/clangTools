//
// Created by caesar kekxv on 2024/6/14.
//
#include <date.hpp>
#include <iostream>
#include <regex>

int main(int argc, const char *argv[]) {
  const auto date1 = clangTools::date::parse(1718372061);
  std::cout << __LINE__ << ":" << date1->format() << std::endl;
  const auto date2 = clangTools::date::parseMicroseconds(1718372061000);
  std::cout << __LINE__ << ":" << date2->format() << std::endl;
  const auto date3 = clangTools::date::parse(2024, 06 - 1, 14);
  std::cout << __LINE__ << ":" << date3->format() << std::endl;
  const auto date4 = clangTools::date::parse(2024, 06 - 1, 32, 8, 59);
  std::cout << " " << date4->format("yyyy-MM-dd HH:mm:ss");
  std::cout << "\t" << "yyyy-MM-dd HH:mm:ss" << std::endl;
  std::cout << " " << *date4;
  std::cout << "\t" << "yyyy-MM-dd HH:mm:ss" << std::endl;
  std::cout << " " << date4->format("yyyy+1-MM-1-dd HH:mm:ss");
  std::cout << "\t" << "yyyy+1-MM-1-dd HH:mm:ss" << std::endl;
  std::cout << " " << date4->format("yyyy--06-dd HH:mm:ss");
  std::cout << "\t" << "yyyy--06-dd HH:mm:ss" << std::endl;
  std::cout << " " << date4->clone("yyyy-1-MM+1-dd HH:mm:ss")->format("yyyy-MM-dd HH:mm:ss");
  std::cout << "\t" << "yyyy-1-MM+1-dd HH:mm:ss" << "_" << "yyyy-MM-dd HH:mm:ss" << std::endl;
  std::cout << " " << date4->format("yyyy--07--02 HH:mm:ss");
  std::cout << "\t" << "yyyy--07--02 HH:mm:ss" << std::endl;
  std::cout << "\t" << date1 << std::endl;
  std::cout << "\t" << date2 << std::endl;
  std::cout << "\t" << (date1 == date2) << std::endl;
  return 0;
}
