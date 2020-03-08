//
// Created by caesar kekxv on 2020/3/7.
//

#include <usb_tool.h>
#include <logger.h>
using namespace clangTools;

int main(int argc,char *argv[]){
    logger::instance()->init_default();
    usb_tool::list_print();
    return 0;
}