
'use strict';

goog.provide('Blockly.JavaScript.junior');

goog.require('Blockly.JavaScript');


Blockly.JavaScript['green_flag'] = function(block) {
    var code = "send('s', 0);";
    return code;
};

Blockly.JavaScript['right'] = function(block) {
  var code = "send('r', 1000);";
  return code;
};

Blockly.JavaScript['left'] = function(block) {
  var code = "send('l', 1000);";
  return code;
};

Blockly.JavaScript['up'] = function(block) {
  var code = "send('u', 1000);";
  return code;
};

Blockly.JavaScript['down'] = function(block) {
  var code = "send('d', 1000);";
  return code;
};

Blockly.JavaScript['turn_left'] = function(block) {
  var code = "send('L', 500);";
  return code;
};

Blockly.JavaScript['turn_right'] = function(block) {
  var code = "send('R', 500);";
  return code;
};

Blockly.JavaScript['end'] = function(block) {
  var code = "send('e', 0);";
  return code;
};


Blockly.JavaScript['stop'] = function(block) {
  var code = "send('t', 0);";
  return code;
};

Blockly.JavaScript['repeat'] = function(block) {
  var code = "send('p', 0);";
  return code;
};

Blockly.JavaScript['right_r'] = function(block) {
  var value_repeat = Blockly.JavaScript.valueToCode(block, 'REPEAT', Blockly.JavaScript.ORDER_ATOMIC);
  if(value_repeat == "") value_repeat = 1;
  // TODO: Assemble JavaScript into code variable.
  value_repeat = eval(value_repeat); //in case the text is something like (3+3)
  var code = "";
  for(var i=0; i<value_repeat; i++)
	  code = code+"send('r', 1000);";
  return code;
};

Blockly.JavaScript['left_r'] = function(block) {
  var value_repeat = Blockly.JavaScript.valueToCode(block, 'REPEAT', Blockly.JavaScript.ORDER_ATOMIC);
  if(value_repeat == "") value_repeat = 1;
  // TODO: Assemble JavaScript into code variable.
  value_repeat = eval(value_repeat); //in case the text is something like (3+3)
  var code = "";
  for(var i=0; i<value_repeat; i++)
	  code = code+"send('l', 1000);";
  return code;
};

Blockly.JavaScript['up_r'] = function(block) {
  var value_repeat = Blockly.JavaScript.valueToCode(block, 'REPEAT', Blockly.JavaScript.ORDER_ATOMIC);
  if(value_repeat == "") value_repeat = 1;
  // TODO: Assemble JavaScript into code variable.
  value_repeat = eval(value_repeat); //in case the text is something like (3+3)
  var code = "";
  for(var i=0; i<value_repeat; i++)
	  code = code+"send('u', 1000);";
  return code;
};

Blockly.JavaScript['down_r'] = function(block) {
  var value_repeat = Blockly.JavaScript.valueToCode(block, 'REPEAT', Blockly.JavaScript.ORDER_ATOMIC);
  if(value_repeat == "") value_repeat = 1;
  // TODO: Assemble JavaScript into code variable.
  value_repeat = eval(value_repeat); //in case the text is something like (3+3)
  var code = "";
  for(var i=0; i<value_repeat; i++)
	  code = code+"send('d', 1000);";
  return code;
};

Blockly.JavaScript['turn_right_r'] = function(block) {
  var value_repeat = Blockly.JavaScript.valueToCode(block, 'REPEAT', Blockly.JavaScript.ORDER_ATOMIC);
  if(value_repeat == "") value_repeat = 1;
  // TODO: Assemble JavaScript into code variable.
  value_repeat = eval(value_repeat); //in case the text is something like (3+3)
  var code = "";
  for(var i=0; i<value_repeat; i++)
	  code = code+"send('R', 500);";
  return code;
};

Blockly.JavaScript['turn_left_r'] = function(block) {
  var value_repeat = Blockly.JavaScript.valueToCode(block, 'REPEAT', Blockly.JavaScript.ORDER_ATOMIC);
  if(value_repeat == "") value_repeat = 1;
  // TODO: Assemble JavaScript into code variable.
  value_repeat = eval(value_repeat); //in case the text is something like (3+3)
  var code = "";
  for(var i=0; i<value_repeat; i++)
	  code = code+"send('L', 500);";
  return code;
};

Blockly.JavaScript['num_1'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '1';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['num_2'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '2';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['num_3'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '3';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['num_4'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '4';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return code;
};

Blockly.JavaScript['num_5'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '5';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['num_6'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '6';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['num_7'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '7';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['num_8'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '8';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['num_9'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '9';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['num_10'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '10';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['dice_1'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '1';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['dice_2'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '2';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['dice_3'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '3';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['dice_4'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '4';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['dice_5'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '5';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['dice_6'] = function(block) {
  // TODO: Assemble JavaScript into code variable.
  var code = '6';
  // TODO: Change ORDER_NONE to the correct strength.
  //return [code, Blockly.JavaScript.ORDER_NONE];
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['plus'] = function(block) {
  var value_first = Blockly.JavaScript.valueToCode(block, 'FIRST', Blockly.JavaScript.ORDER_ATOMIC);
  var value_second = Blockly.JavaScript.valueToCode(block, 'SECOND', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = parseInt(value_first)+parseInt(value_second);
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['minus'] = function(block) {
  var value_first = Blockly.JavaScript.valueToCode(block, 'FIRST', Blockly.JavaScript.ORDER_ATOMIC);
  var value_second = Blockly.JavaScript.valueToCode(block, 'SECOND', Blockly.JavaScript.ORDER_ATOMIC);
  // TODO: Assemble JavaScript into code variable.
  var code = parseInt(value_first)-parseInt(value_second);
  // TODO: Change ORDER_NONE to the correct strength.
  return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['repeat_loop'] = function(block) {
  var value_times = Blockly.JavaScript.valueToCode(block, 'TIMES', Blockly.JavaScript.ORDER_ATOMIC);
  var statements_do = Blockly.JavaScript.statementToCode(block, 'DO');
  // TODO: Assemble JavaScript into code variable.
  var code = '';
  for(var i=0; i<value_times; i++)
  {
	  code = code+statements_do;
  }
  return code;
};

