/**
* @file Sequencer.h
* @brief 
* @author 10gic
* @date 2011/5/9
* @version 
* @note
* 这里没有定义类，只是为了main函数简单，把一些操作封装成了函数sequencer。
*/

#ifndef SEQUENCER_H
#define SEQUENCER_H

extern uint space;

//功能：按优先级顺序将URL加入到namedSiteList中，同时控制URL的数量。
void sequencer();

#endif