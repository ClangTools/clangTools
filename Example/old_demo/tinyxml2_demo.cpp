//
// Created by caesar on 2019/11/28.
//
#include <tinyxml2.h>
#include <string>
using namespace std;

int main(int argc,char**argv){
    tinyxml2::XMLDocument xml{};
    string xmlParam = "<?xml version = \"1.0\" encoding=\"utf-8\" ?>"
                             "<param>"
                             "<imgWidth>640</imgWidth>"
                             "<imgHeight>480</imgHeight>"
                             "<roiX>0</roiX>"
                             "<roiY>0</roiY>"
                             "<roiW>640</roiW>"
                             "<roiH>480</roiH>"
                             "</param>";
    xml.Parse(xmlParam.c_str(), xmlParam.size());
    printf("%s\n",xml.FirstChildElement("param")->FirstChildElement("imgWidth")->GetText());
    xml.FirstChildElement("param")->FirstChildElement("imgWidth")->SetText("1024");
    printf("%s\n",xml.FirstChildElement("param")->FirstChildElement("imgWidth")->GetText());


    tinyxml2::XMLPrinter stream;
    xml.Print(&stream);

    printf("xmlParam:%s\n", xmlParam.c_str());
    printf("stream:%s\n", stream.CStr());
    return 0;
}
