// https://www.keithmcmillen.com/blog/making-music-in-the-browser-web-midi-api/
// https://webaudio.github.io/web-midi-api/#handling-midi-input

var theUniverse = null;

var numRows = 2,
	numCols = 16;
var data, cmd, channel, type, note, velocity;

var ccs = [21,22,23,24,7];


var selectMIDIIn = null;
var selectMIDIOut = null;
var midiAccess = null; // global MIDIAccess object
var midiIn = null;
var midiOut = null;
var omx27 = false;

var notes = [[0, 61,63, 0, 66,68,70, 0, 73,75, 0, 78,80,82,0,0],[59,60,62,64,65,67,69,71,72,74,76,77,79,81,83,84]];

var count = ccs.length;;


window.addEventListener('keydown', function() { 
} );

window.addEventListener('load', function() {
	theUniverse = document.getElementById("universe");
	
	var t = document.createElement("div");
	t.setAttribute("name", "grid");
	
	var z = 59;
	for (var row = 0; row < numRows; row++) {
		
		var newRow = document.createElement("div");
		newRow.className = "row"+row;
		newRow.row = row;
		//console.log(newRow);
		t.appendChild(newRow);
		
		for (var col = 0; col < numCols; col++) {
			var newCell = document.createElement("div");
			newRow.appendChild(newCell);
			newCell.row = row;
			newCell.col = col;
			newCell.onclick = flipHandler;
			newCell.setAttribute("id", notes[row][col]);

			if (notes[row][col] == 0){
				newCell.className = "blankcell";
			} else { 
				newCell.className = "cell";
				newCell.innerHTML=notes[row][col]; 
			}
			newRow.appendChild(newCell);
			z = z+1;
		}
	}
	document.body.appendChild(t);
} );
    
    
window.addEventListener("DOMContentLoaded", function(event) {
  if (navigator.requestMIDIAccess) {
	  console.log('This browser supports WebMIDI!');
  } else {
	  console.log('WebMIDI is not supported in this browser.');
	  document.querySelector("#midi").style.display = "none";
	  document.querySelector("#nomidi").style.display = "block";
  }
//	navigator.requestMIDIAccess({sysex: true}).then( onMIDIInit, onMIDIFail );
	navigator.requestMIDIAccess({}).then( onMIDIInit, onMIDIFail );
      
      for(var i = 0; i < count; i++) {
        var slider = document.querySelectorAll('.slider')[i]
        slider.style.height = 1 + "px";
        slider.style.width = 20 + "px";
        slider.style.top = 128 + "px";
        slider.style.left = (i*25)+5 + "px";

        var label = document.querySelectorAll('#labels div')[i];
        label.innerHTML = i+1;
        label.style.left = (i*25)+10 + "px";
        label.style.top = "135px";
      }

});



function changeMIDIIn( ev ) {
  if (midiIn)
    midiIn.onmidimessage = null;
  var selectedID = selectMIDIIn[selectMIDIIn.selectedIndex].value;

  for (var input of midiAccess.inputs.values()) {
    if (selectedID == input.id)
      midiIn = input;
  }
  midiIn.onmidimessage = midiProcess;
}

function changeMIDIOut( ev ) {
  var selectedID = selectMIDIOut[selectMIDIOut.selectedIndex].value;

  for (var output of midiAccess.outputs.values()) {
    if (selectedID == output.id) {
      midiOut = output;
      console.log('changed to output ' + output.name);
	}
  }
}


function onMIDIInit(midi) {
	midiAccess = midi;

  //var inputs = midiAccess.inputs;
  //var outputs = midiAccess.outputs;
  
  selectMIDIIn=document.getElementById("midiIn");
  selectMIDIOut=document.getElementById("midiOut");
  console.log('WebMIDI init');

  for (var entry of midiAccess.inputs) {
  	var item = entry[1];
  	console.log( "Input port [type:'" + item.type + "'] id:'" + item.id + "' manufacturer:'" + item.manufacturer + "' name:'" + item.name + "' version:'" + item.version + "'" );
  }

  // clear the MIDI input select
  selectMIDIIn.options.length = 0;
  for (var input of midiAccess.inputs.values()) {
    if ((input.name.toString().indexOf("omx-27") != -1)) {
      omx27 = true;
      selectMIDIIn.add(new Option(input.name,input.id,true,true));
      midiIn=input;
	  midiIn.onmidimessage = midiProcess;

    }else{
  		selectMIDIIn.add(new Option(input.name,input.id,false,false));
  	}
  }
  selectMIDIIn.onchange = changeMIDIIn;

  // clear the MIDI output select
  selectMIDIOut.options.length = 0;
  for (var output of midiAccess.outputs.values()) {
    if ((output.name.toString().indexOf("omx-27") != -1)) {
      selectMIDIOut.add(new Option(output.name,output.id,true,true));
      midiOut=output;
    }else{
	    selectMIDIOut.add(new Option(output.name,output.id,false,false));
	}
  }
  selectMIDIOut.onchange = changeMIDIOut;


}

function onMIDIFail() {
  console.log('Could not access your MIDI devices.');
}

function onMIDISuccess(midiAccess) {
	for (var input of midiAccess.inputs.values()) {
		input.onmidimessage = midiProcess;
	}
}

function midiProcess(ev) {
    data = ev.data,
    cmd = data[0] >> 4,
    channel = data[0] & 0xf,
    type = data[0] & 0xf0, // channel agnostic message type. Thanks, Phil Burk.
    note = data[1],
    velocity = data[2];
    // with pressure and tilt off
    // note off: 128, cmd: 8 
    // note on: 144, cmd: 9
    // pressure / tilt on
    // pressure: 176, cmd 11: 
    // bend: 224, cmd: 14

    switch (type) {
		case 144: // noteOn message 
        	if (velocity != 0){
        		noteOn(note, velocity);
        	}else{
        		noteOff(note, velocity); 
        	}
        	console.log('noteOn: '+ note);
            break;
		case 128: // noteOff message 
            noteOff(note, velocity);
            console.log('noteOff: '+ note);
            break;
		case 176: // CC message 
			controller( note, velocity)
            console.log('cc: '+ note);
            break;
		case 224: // pitchbend message 
			//pitchWheel( ((velocity * 128.0 + note)-8192)/8192.0 );
            console.log('pitchbend: '+ note);
            break;
        case 240: // sysex
        	break;
        	
  }

	
	if(cmd>14){ //handle sysex - type 240

		if(ev.data[5]===0 && ev.data[6]===1 && ev.data[7]===97){
			var id = ev.data[10];
			//product = products[id]; //turn product ID into a name
			clog('product: (id '+id+')');
		}

	}
}

function $(id) {
	return document.getElementById(id);
}

function noteOn( note, velocity ) {
	//console.log(' note on '+note+' '+velocity);
	$('notenum').innerHTML = note;
	$('noteval').innerHTML = velocity;
	$(note).className = "cell on";
	
}

function noteOff( note ) {
	//console.log('note off '+note);
	$('notenum').innerHTML = note;
	$('noteval').innerHTML = 0;
	$(note).className = "cell";
	
}

function controller( number, value ) {
	//console.log('cc '+number+' '+value);
	$('ccnum').innerHTML = number;
	$('ccval').innerHTML = value;
	  //send back to controller to update LEDs
	  //ring(number,value);
	  var index = ccs.indexOf(number);
	  if (index != -1) {
		document.querySelectorAll('.slider')[index].style.height = value + 1 + "px"; 
		document.querySelectorAll('.slider')[index].style.top = 129-value + "px"; 
	  }

}

var currentPitchWheel = 0.0;
// 'value' is normalized to [-1,1]
function pitchWheel( value ) {
	var i;
	currentPitchWheel = value;
	console.log('pitchwheel '+value);
}

function flipHandler(e) {
	//console.log(e.target.id);
	flip( e.target.id );
}

function flip(note) {
	if ($(note).className == "cell"){  // dead
		$(note).className = "cell on";
		midiOut.send( [0x90, note, 0x7f] );
	}else{
		$(note).className = "cell";
		midiOut.send( [0x80, note, 0x40] );
	}
	//var key = elem.row*16 + elem.col;
	//midiOut.send( [0x90, elem, elem.classList.contains("on") ? (0x80) : 0x00]);
}
