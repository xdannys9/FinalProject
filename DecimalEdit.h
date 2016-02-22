/*
 * DecimalEdit.h
 *
 *  Created on: 11.02.2016
 *      Author: georgrokita
 */

#ifndef DECIMALEDIT_H_
#define DECIMALEDIT_H_

#include "PropertyEdit.h"
#include "ModbusMaster.h"
#include <string>
#include "LiquidCrystal.h"

void Sleep(int ms);


class DecimalEdit:public PropertyEdit {
public:
	DecimalEdit(LiquidCrystal& lcd_,ModbusMaster& node_, std::string editTitle, float _upperL = 100,float _lowerL =0);
	virtual ~DecimalEdit();
	void increment();
	void decrement();
	void accept();
	void cancel();
	void setFocus(bool focus);
	void display();
	int getValue();
	void setValue(float value);
private:
	void save();
	void displayEditValue();
	LiquidCrystal& lcd;
	ModbusMaster& node;
	std::string title;
	float value;
	float edit;
	bool focus;
	float upperL;
	float lowerL;
};

#endif /* DECIMALEDIT_H_ */
