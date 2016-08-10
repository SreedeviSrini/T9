    
function createNaClModule() {
    console.log('Create module called');
    var moduleEl = document.createElement('embed');
    moduleEl.setAttribute('name', 'hello_tutorial');
    moduleEl.setAttribute('id', 'hello_tutorial');
    moduleEl.setAttribute('width', 0);
    moduleEl.setAttribute('height',0);
    moduleEl.setAttribute('src', 'hello_tutorial.nmf');
    moduleEl.setAttribute('type', 'application/x-nacl');

    // The <EMBED> element is wrapped inside a <DIV>, which has both a 'load'
    // and a 'message' event listener attached.  This wrapping method is used
    // instead of attaching the event listeners directly to the <EMBED> element
    // to ensure that the listeners are active before the NaCl module 'load'
    // event fires.
    var listenerDiv = document.getElementById('listener');
    listenerDiv.appendChild(moduleEl);
  }

    function pageDidLoad() {
	  console.log('Page Did load');
	 

      if (HelloTutorialModule == null) {
	createNaClModule();
        updateStatus('LOADING...');

      HelloTutorialModule = document.getElementById('hello_tutorial');
      
      updateStatus('SUCCESS');
      } else {
        updateStatus();
      }
    }

    function updateStatus(opt_message) {
      if (opt_message)
      {
        statusText =  opt_message;
      }
      var statusField = document.getElementById('status_field');
      if (statusField) {
        statusField.innerHTML = statusText;
      }
    }
	function add(str) {
		var wordl = document.getElementById('wordlist');

		var objOption = document.createElement("option");

		for(var i=0, len=str.length; i<len; i++) {
			objOption.text = str[i];
			//console.log(objOption.text);
			wordl.options[i] = new Option(objOption.text, objOption.text);
		}
	}


    var wordPresent = 'False';
    function updateWords(opt_message) {
        statusText =  opt_message;
		console.log("word is "+opt_message);
		if (opt_message != "State Changed")
		{
			if ( opt_message.indexOf('noword') == -1 )
			{
				var n = opt_message.split(" ");
				//console.log('Space at ' + n);
				if (n.length)
				{
				var wordl = document.getElementById('wordlist');
				wordl.options.length = 0;
				add(n);
				statusText = n[0];
				wordPresent = 'True';
				}
				document.getElementById('predict').value= statusText;
			}
			else
			{
				wordPresent = 'False';
			}
		}
    }
	
	

    function sendNumber(evt,keys,letter) {
           if( fileSelected == false )
	   {
		  alert('Dictionary not loaded');
		  return;
	   }

	  console.log('sendNumber');
	  if ( keys == 'c' )
	  {
	       document.getElementById('predict').value = "";
	  	   HelloTutorialModule.postMessage(' ');  //To reset to head 
		   var wordl = document.getElementById('wordlist');
		   wordl.options.length = 0;
	  }
	  else
	  {

		    var oldVal = document.getElementById('predict').value;
		    document.getElementById('predict').value= oldVal + letter;	
		    if ( keys == '1' )
			    {
			    var splchar = new Array("?","!","#","$",",","@","-","_");
			    var wordl = document.getElementById('wordlist');
			    wordl.options.length = 0;
			    add(splchar);
			    return;
			    }

		    //console.log(keys);

		  if ( keys == ' ' )
		  {
			var wordl = document.getElementById('wordlist');
			wordl.options.length = 0;
	  		var statusField = document.getElementById('word_field');
		      	if (statusField) {

				var str = document.getElementById('predict').value;
				console.log(str);
				if (( (str.indexOf('?') != -1) && str[0] == '?')
				|| ( (str.indexOf('$') != -1) && str[0] == '$')
				|| ( (str.indexOf(',') != -1) && str[0] == ',')
				|| ( (str.indexOf('@') != -1) && str[0] == '@')
				|| ( (str.indexOf('-') != -1) && str[0] == '-')
				|| ( (str.indexOf('_') != -1) && str[0] == '_')
				|| ( (str.indexOf('#') != -1) && str[0] == '#')
				|| ( (str.indexOf('!') != -1) && str[0] == '!'))
				{
					console.log(str);
    			    		statusField.innerHTML += ' ';
					statusField.innerHTML += document.getElementById('predict').value;
	  	       	        	document.getElementById('predict').value = "";
					return;
				}

				if ( wordPresent == 'True' )
				{
    			    	   statusField.innerHTML += ' ';
				   statusField.innerHTML += document.getElementById('predict').value;
				   var reg = /[^a-zA-Z]/g;
				   var index = str.search(reg);
				   var strword;
				   if(index != -1)
				   {
				   	strword = str.slice(0,index);
				   }
				   else
				   {
					strword = str;
				   }
				   HelloTutorialModule.postMessage('UPDATE '+strword);  // Global application object.
				}
				else
				{
				 if(confirm('Not a word!- Do you want to spell now?'))
				  {
					  var uw = document.getElementById('userword');
					  var ui = document.createElement("input");
					  ui.type = "text";
					  ui.id = "uwi";
					  uw.appendChild(ui);
					  var ub = document.createElement("input");
					  ub.type = "button";
					  ub.value = 'Add Word';
					  ub.id = "ub";
					  uw.appendChild(ub);
					  
   					  document.getElementById("ub").addEventListener('click', function() 
					  { 
						  console.log('Add word clicked');
						  if ( document.getElementById('uwi').value )
					          {
						  var userstring = document.getElementById('uwi').value;
						  console.log(document.getElementById('uwi').value);
						  var reg = /[^a-zA-Z]/g;
						  var index = userstring.search(reg);
						  var strword;
						  if(index != -1)
						  {
							strword = userstring.slice(0,index);
						  }
						  else
						  {
							strword = userstring;
						  }
						  console.log('strword is'+strword);
					  	  HelloTutorialModule.postMessage('ADD '+ strword);  // Global application object.
      	      					  var statusField = document.getElementById('word_field');
				                  statusField.innerHTML += ' ';
				    		  statusField.innerHTML += document.getElementById('uwi').value;
					 	  }
					  	  else
						  {
					          alert('User Word Empty');
						  }
				          },true);

			       	  }
				  else
				  {
					  console.log("No dont add to dict");
				  }	
				}
	  	       	        document.getElementById('predict').value = "";
    	  		}
	  	        HelloTutorialModule.postMessage(keys);  // Global application object.
		  }
		  else
		  {
	  	    HelloTutorialModule.postMessage(keys);  // Global application object.
		  }
	  }

    }

  var fileSelected = false;
  
  function readBlob() {

    var files = document.getElementById('files').files;
    if (!files.length) {
      alert('Please select a file!');
      return;
    }


    var file = files[0];

    var reader = new FileReader();

    // If we use onloadend, we need to check the readyState.
    reader.onloadend = function(evt) {

	if (evt.target.readyState == FileReader.DONE)
	{ // DONE == 2
    		fileSelected = true;

		var str;
		evt.target.result += ',';
	
		HelloTutorialModule.postMessage(evt.target.result);  // Global application object.

		//str = ','
		console.log("Loading Done");
		HelloTutorialModule.postMessage('Loading Done');  // Global application object.
	}
    };

    reader.readAsBinaryString(file);
    console.log('Going to register buttons'); 

   document.getElementById("btn1").addEventListener('click', function(evt) {sendNumber(evt,'1','?')},true);
   document.getElementById("btn2").addEventListener('click', function(evt) {sendNumber(evt,'2','a')},true);
   document.getElementById("btn3").addEventListener('click', function(evt) {sendNumber(evt,'3','e')},true);
   document.getElementById("btn4").addEventListener('click', function(evt) {sendNumber(evt,'4','i')},true);
   document.getElementById("btn5").addEventListener('click', function(evt) {sendNumber(evt,'5','k')},true);
   document.getElementById("btn6").addEventListener('click', function(evt) {sendNumber(evt,'6','m')},true);
   document.getElementById("btn7").addEventListener('click', function(evt) {sendNumber(evt,'7','s')},true);
   document.getElementById("btn8").addEventListener('click', function(evt) {sendNumber(evt,'8','u')},true);
   document.getElementById("btn9").addEventListener('click', function(evt) {sendNumber(evt,'9','y')},true);
   document.getElementById("btns").addEventListener('click', function(evt) {sendNumber(evt,' ',' ')},true);
   document.getElementById("btnc").addEventListener('click', function(evt) {sendNumber(evt,'c','c')},true);

  }
  
  
  
  window.addEventListener("load", pageDidLoad,true);
  HelloTutorialModule = null;  // Global application object.
  statusText = 'NO-STATUS';
   document.querySelector('.readBytesButtons').addEventListener('click', function(evt) {
    if (evt.target.tagName.toLowerCase() == 'button') {
	readBlob();
    }
   
   document.getElementById("wordlist").addEventListener('change', function() {
	   if ( (this.options[this.selectedIndex].value == '#') ||
	        (this.options[this.selectedIndex].value == '$') ||
	        (this.options[this.selectedIndex].value == '-') ||
	        (this.options[this.selectedIndex].value == '_') ||
	        (this.options[this.selectedIndex].value == ',') ||
	        (this.options[this.selectedIndex].value == '@') ||
	        (this.options[this.selectedIndex].value == '?') ||
	        (this.options[this.selectedIndex].value == '!') ) 
	   {
	   var str = document.getElementById('predict').value.slice(0, document.getElementById('predict').value.length -1);
	   console.log('String is '  + str + 'Length: ' + document.getElementById('predict').value.length);
	   str+= this.options[this.selectedIndex].value;
	   document.getElementById('predict').value= str;
	   }
	   else
   	   {
	   document.getElementById('predict').value= this.options[this.selectedIndex].value;
	   }
   },true);

   var listenerDiv = document.getElementById('listener');
   listenerDiv.addEventListener('message', handleMessage, true);
  }, false);


function handleMessage(message_event) {
      //console.log(message_event.data);
      if (message_event.data.indexOf('Dbg') != -1 )
      {
	      updateStatus('Load Success! - Ready for Prediction');
      }
      else if (message_event.data.indexOf('WordAdded') != -1 )
      {
	      console.log('Word added to Dictionary');
	      var uw = document.getElementById('userword');
	      uw.removeChild(uwi);
	      uw.removeChild(ub);
	      //updateStatus('Load Success! - Ready for Prediction');
      }
      else
      {
	      updateWords(message_event.data);
      }
    }
