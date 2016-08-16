module.exports = function(minified) {
  var Clay = this;
  var _ = minified._;
  var $ = minified.$;
  var HTML = minified.HTML;
  var clayConfig = this;
  
 function toggleChoiceText() {
    // Default to hiding all the text choice
    clayConfig.getItemById('Location').hide();
    clayConfig.getItemById('Conditions').hide();
 
    switch (this.get()) {
      case '1': 
        clayConfig.getItemById('Location').show();
        break;
      case '2': 
        clayConfig.getItemById('Conditions').show();
        break;
    }
  }
  
   function toggleChoiceText1() {
    // Default to hiding all the text choice
    clayConfig.getItemById('Location').hide();
    clayConfig.getItemById('Conditions').hide();
    clayConfig.getItemById('CustomText').hide();
 
    switch (this.get()) {
      case '1': 
        clayConfig.getItemById('Location').show();
        break;
      case '2': 
        clayConfig.getItemById('Conditions').show();
        break;
      case '13': 
        clayConfig.getItemById('CustomText').show();
        break;
    }
  }
  
     function toggleChoiceText2() {
    // Default to hiding all the text choice
    clayConfig.getItemById('LowBatteryLevel').hide();
 
    switch (this.get()) {
      case true: 
        clayConfig.getItemById('LowBatteryLevel').show();
        break;
    }
  }
  
  Clay.on(Clay.EVENTS.AFTER_BUILD, function() {    
       
 // Trigger choice text
    var ChoiceToggle = clayConfig.getItemByMessageKey('TOP_INFORMATION_BOX');
    toggleChoiceText.call(ChoiceToggle);
    ChoiceToggle.on('change', toggleChoiceText);
    
    var ChoiceToggle1 = clayConfig.getItemByMessageKey('BOTTOM_INFORMATION_BOX');
    toggleChoiceText1.call(ChoiceToggle1);
    ChoiceToggle1.on('change', toggleChoiceText1);  
    
    var ChoiceToggle2 = clayConfig.getItemByMessageKey('LOW_BATTERY_MODE');
    toggleChoiceText2.call(ChoiceToggle2);
    ChoiceToggle2.on('change', toggleChoiceText2); 
    
    
    var connection = new WebSocket("wss://liveconfig.fletchto99.com/forward/" + Clay.meta.accountToken + "/" + Clay.meta.watchToken);
    connection.onopen = function() {
      Clay.getAllItems().map( function(item) {
        item.on('change', function() {
          connection.send(JSON.stringify({id: this.messageKey, value: this.get()}));//appKey
        });
      });
    };
  });
};
