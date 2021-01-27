Blockly.Blocks['green_flag'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_green_flag.png", 32, 32, "green_flag"));
    this.setNextStatement(true, "JUNIOR");
    this.setColour(60);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['right'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_arrow_right.png", 32, 32, "right"));
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['left'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_arrow_left.png", 32, 32, "left"));
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['up'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img//jr_arrow_up.png", 32, 32, "up"));
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['down'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_arrow_down.png", 32, 32, "down"));
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['turn_right'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_turn_right.png", 32, 32, "turn_right"));
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['turn_left'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_turn_left.png", 32, 32, "turn_left"));
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['end'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_empty.png", 32, 32, "end"));
    this.setPreviousStatement(true, "JUNIOR");
    this.setColour(0);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['stop'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_stop7.png", 32, 32, "stop"));
    this.setPreviousStatement(true, "JUNIOR");
    this.setColour(0);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['repeat'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_repeat.png", 32, 32, "repeat"));
    this.setPreviousStatement(true, "JUNIOR");
    this.setColour(0);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['right_r'] = {
  init: function() {
    this.appendValueInput("REPEAT")
        .setCheck("Number")
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr_arrow_right.png", 32, 32, "right"));
    this.setInputsInline(false);
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['left_r'] = {
  init: function() {
    this.appendValueInput("REPEAT")
        .setCheck("Number")
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr_arrow_left.png", 32, 32, "left"));
    this.setInputsInline(false);
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['up_r'] = {
  init: function() {
    this.appendValueInput("REPEAT")
        .setCheck("Number")
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr_arrow_up.png", 32, 32, "up"));
    this.setInputsInline(false);
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['down_r'] = {
  init: function() {
    this.appendValueInput("REPEAT")
        .setCheck("Number")
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr_arrow_down.png", 32, 32, "down"));
    this.setInputsInline(false);
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['turn_right_r'] = {
  init: function() {
    this.appendValueInput("REPEAT")
        .setCheck("Number")
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr_turn_right.png", 32, 32, "turn_right"));
    this.setInputsInline(false);
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['turn_left_r'] = {
  init: function() {
    this.appendValueInput("REPEAT")
        .setCheck("Number")
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr_turn_left.png", 32, 32, "turn_left"));
    this.setInputsInline(false);
    this.setPreviousStatement(true, "JUNIOR");
    this.setNextStatement(true, "JUNIOR");
    this.setColour(195);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_1'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("1");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_2'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("2");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_3'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("3");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_4'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("4");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_5'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("5");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_6'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("6");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_7'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("7");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_8'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("8");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_9'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("9");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['num_10'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("10");
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['dice_1'] = {
  init: function() {
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr-dice-1.png", 32, 32, "1"));
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['dice_2'] = {
  init: function() {
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr-dice-2.png", 32, 32, "1"));
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['dice_3'] = {
  init: function() {
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr-dice-3.png", 32, 32, "1"));
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['dice_4'] = {
  init: function() {
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr-dice-4.png", 32, 32, "1"));
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['dice_5'] = {
  init: function() {
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr-dice-5.png", 32, 32, "1"));
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['dice_6'] = {
  init: function() {
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_CENTRE)
        .appendField(new Blockly.FieldImage("img/jr-dice-6.png", 32, 32, "1"));
    this.setOutput(true, "Number");
    this.setColour(270);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['plus'] = {
  init: function() {
    this.appendValueInput("FIRST")
        .setCheck("Number");
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_plus.png", 32, 32, "*"));
    this.appendValueInput("SECOND")
        .setCheck("Number");
    this.setInputsInline(true);
    this.setOutput(true, "Number");
    this.setColour(225);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['minus'] = {
  init: function() {
    this.appendValueInput("FIRST")
        .setCheck("Number");
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_minus.png", 32, 32, "*"));
    this.appendValueInput("SECOND")
        .setCheck("Number");
    this.setInputsInline(true);
    this.setOutput(true, "Number");
    this.setColour(225);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['repeat_loop'] = {
  init: function() {
    this.appendDummyInput()
        .appendField(new Blockly.FieldImage("img/jr_repeat.png", 32, 32, "*"));
    this.appendValueInput("TIMES")
        .setCheck("Number");
    this.appendStatementInput("DO");
    this.setInputsInline(true);
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(30);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

