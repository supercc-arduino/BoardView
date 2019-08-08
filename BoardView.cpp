#include <BoardView.h>

int (*boardViewParseRequest) (char *request, char *response, unsigned len);

WebSocketsServer *boardViewWebSocketServer;
Stream *boardViewRedirectStream;
unsigned boardViewMaxLineLen;

static void boardViewEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){ 
	
	if(!length) return; // message vide ignoré
	 
	if (type == WStype_TEXT){  // on ne joue qu'avec du "texte".
		
		char response[boardViewMaxLineLen]; // de l'espace pour la réponse
		
		// mode direct ?          	          
		if(boardViewRedirectStream==NULL) {  
			
			// mode direct, les messages sont traités localement
			
			#ifdef BOARD_VIEW_DEBUG
			if(boardViewRedirectStream == NULL) {
				Serial.print("recv :");
				Serial.print((char *)payload);
				Serial.println(".");
			}
			#endif
			 		          		          
			boardViewParseRequest((char *)payload, response, boardViewMaxLineLen-1 );
			strcat(response,"\n");
			
			#ifdef BOARD_VIEW_DEBUG
			if(boardViewRedirectStream == NULL) {
				Serial.print("response :");
				Serial.print(response);
				Serial.println(".");
			}
			#endif
			
			boardViewWebSocketServer->sendTXT(num, response, strlen(response));
		}
		else {
			// mode "redirect"
			//
			// on vient de recevoir une requête réseau (webSocket),
			// on l'envoie sur le flux destination.
			
			boardViewRedirectStream->println((char *)payload);
	
			boardViewRedirectStream->flush();	
			
			// On attend la réponse
			
			while(!boardViewRedirectStream->available()) ;
			
			int n;
			n=boardViewRedirectStream->readBytesUntil('\n', response, boardViewMaxLineLen);
			response[n]=0; 
					
			// remove trailling '\n' or '\r\n';
			int e=strlen(response)-1; 
			if(response[e]=='\r') { response[e--]=0; }
			if(response[e]=='\n') response[e]=0; 
			 
			// On émet la réponse au demandeur (réseau)
		    strcat(response, "\n");
		    
		    boardViewWebSocketServer->sendTXT(num, response, strlen(response));	
		}
	} 
} 

void BoardView::buildCommon() {
	
	header = "<!DOCTYPE html> \n\
	  <html lang='fr_FR'> \n\
	  <head> \n\
		<meta charset=\"UTF-8\"> \n\
	  </head> \n\
	\n";

	footer = "\n\
	</body> \n\
	</html> \n\
	";

	title = (String)"<h1>"+ name + "</h1>\n";

	styles = "\n\
    <style> \n\
	html { \n\n\
		font-size:"+ String(fontSize) + "rem; \n\
	} \n\
    </style> \n\
    ";
    
    javaScriptCommon = "\n\
    <script> \n\
    \n \
    function enterPressedOnConsole() { \n\
		Socket.send(document.getElementById('txbuff').value); \n\
		document.getElementById('txbuff').value=''; \n\
	} \n\
    \n \n\
    function entryPressed(name){ \n\
			Socket.send(name+'='+document.getElementById(name).value+'\\n'); \n\
	} \n\
	\n \n\
	function checkPressed(name, cmdWhenUnckeck, cmdWhenCkeck) { \n\
		  var w=document.getElementById(name); \n\
		  if(w) { \n\
				if(w.checked) { \n\
					Socket.send(cmdWhenCkeck); \n\
				} \n\
				else { \n\
					Socket.send(cmdWhenUnckeck); \n\
				} \n\
		  } \n\
	  } \n\
	\n \n\
	function limitTextarea(textarea, maxLines) { \n\
		  var lines = textarea.value.replace(/\\r/g, '').split('\\n'); \n\
		  if(lines.length > " + String(maxHistoryLines) +") { \n\
			  while (lines.length > "+ String(maxHistoryLines) + ") { \n\
				  lines.shift(); \n\
			  } \n\
			textarea.value=lines.join('\\n'); \n\
		  } \n\
	  } \n\
    \n\
    </script> \n\
    \n";
}

void BoardView::buildMainPage(String &s) {
	s = header + styles + "\n\
	" + title + " \n\
	<div max-width:90%\"> \n\
	<ul> \n\
	<li><a href='console.html' style='font-size:"+String(fontSize)+"rem'>Console</a></li> \n\
	<li><a href='view.html' style='font-size:"+String(fontSize)+"rem'>View</a></li> \n\
	</ul> \n\
	</div>\n" + footer;
}

void BoardView::buildConsolePage(String &s) {
	s = header + styles + javaScriptCommon + "\n\
	  <script> \n\
	  var socket; \n\
	  function start() { \n\
		  Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); \n\
			  Socket.onmessage=function(evt){ \n\
					limitTextarea(document.getElementById(\"rxConsole\"),100); \n\
					var textarea=document.getElementById(\"rxConsole\"); \n\
					textarea.value += evt.data; \n\
					console.log(textarea.scrollTop); \n\
					console.log(textarea.scrollHeight); \n\
					if( (textarea.scrollHeight-textarea.scrollTop) < 200) \n\
						textarea.scrollTop = textarea.scrollHeight; \n\
			  } \n\
	  } \n\
	  \n\
	  </script> \n\
	  <body onload='javascript:start();'> \n\
	  " + title + "\n\
	  <div style='width:90%'> \n\
	  <textarea id='rxConsole' style='font-size:"+String(fontSize)+"rem; width:100%' \n\
		rows=5 readonly ></textarea> \n\
	  <br><br> \n\
	   \n\
	  <input type='text' id='txbuff' style='font-size:"+String(fontSize)+"rem; width:70%' \n\
		onkeydown='if(event.keyCode==13) enterPressedOnConsole();'> \n\
	  <input type='button' style='font-size:"+String(fontSize)+"rem; width:25%' \n\
		onclick='enterPressedOnConsole();' value=\"Send\" > \n\
	  </div>\n\
	  \n" + footer;
}
 
void BoardView::buildViewPage(String &s) {
		
	s=header+styles + javaScriptCommon + \
	"<script> \n\
	  var intervalID; \n\
	  var Socket; \n\
	  \n\
	  function start() { \n\
		intervalID = window.setInterval(periodicDump, "+viewRefreshPeriodMs+"); \n\
		Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); \n\
		Socket.onmessage=function(evt){ \n\
			// show in console \n\
			textarea=document.getElementById('rxConsole'); \n\n\
			if(textarea) { \n\
				limitTextarea(textarea, "+ String(maxHistoryLines) + ");  \n\
				document.getElementById('rxConsole').value += evt.data;  \n\
				\n\
				// scroll console \n\
				if( (textarea.scrollHeight-textarea.scrollTop) < 400) { \n\
					textarea.scrollTop = textarea.scrollHeight; \n\
				} \n\
			} \n\
			\n\
			// split dump \n\
		    \n\
			evt.data.split(';').forEach(function(e) {  \n\
				\n\
				e=e.split('='); \n\
				 \n\
				if(e.length==2) { \n\
					e[0] = e[0].replace(/^ /, ''); \n\
					var w=document.getElementById(e[0]); \n\
					if(w) { \n\
						if(w.type == 'text') { \n\
							e[1] = e[1].replace(/\"/g, ''); \n\
							w.value=e[1]; \n\
						} \n\
						else if (w.type == 'checkbox') { \n\
							if(e[1] == 0) { \n\
								w.checked=false; \n\
							} \n\
							else { \n\
								w.checked=true; \n\
							} \n\
						}  \n\
					} \n\
				} \n\
			}); \n\
		 } \n\
	  } \n\
	  \n\
	  function periodicDump() { \n\
		  Socket.send('dump'); \n\
	  } \n\
	  \n\
	  function buttonPressed(cmd) { \n\
			cmd.split(';').forEach(function(e) {  \n\
			e=e.replace(/^ */, ''); \n\
			Socket.send(e);  \n\
		}); \n\
	} \n\
	  \n\
	  function promptForInput(name) { \n\
			var input=document.getElementById(name); \n\
			if(input!=null) { \n\
				var newValue = prompt(name, input.value); \n\
				if (newValue != null) { \n\
					input.value=newValue; \n\
					Socket.send(name"+"+\"=\"+"+"newValue); \n\
				} \n\
			} \n\
		} \n\
	  </script>\n\
	  <body onload=\"javascript:start();\">\n\
	  \n\
	  "+title;
	
	s+="<table style='width:90%' cellspacing='30' cellpading='30' >\n";
	for(unsigned int i=0; i<widgetsCpt; ++i)
		s+="<tr>"+widgets[i]->toHtml()+"</tr>\n";
	s+="</table>\n";
	s+=footer;
}
 			  
BoardView::BoardView() {
	
	name="noname";
	
	maxWidgets=32;
	widgets=NULL;
	
	widgetsCpt=0;
	boardViewRedirectStream=NULL;
	httpPort=80;
	webSocketPort=8081;
	tcpPort=83;
	boardViewMaxLineLen=200;
	maxHistoryLines=200;
	
	fontSize=2.0;
	
	viewRefreshPeriodMs=200;
	
}

BoardView::~BoardView() {
	delete boardViewWebSocketServer;
	delete httpServer;
	for (int i=0; i<widgetsCpt; ++i) delete widgets[i];
	delete [] widgets;
}

void BoardView::loop() { 
	boardViewWebSocketServer->loop(); 
	httpServer->handleClient();	
}

void BoardView::begin() { 
		
	boardViewWebSocketServer = new WebSocketsServer(webSocketPort);
	httpServer = new ESP8266WebServer(httpPort);
	
	httpServer->on("/", [&]() {
		buildCommon();
		buildMainPage(mainPage);
		httpServer->send(200, "text/html", mainPage);
	});
	
	httpServer->on("/console.html", [&]() {
		buildCommon();
		buildConsolePage(consolePage);
		httpServer->send(200, "text/html", consolePage);
	});
	
	httpServer->on("/view.html", [&]() {
		buildCommon();
		buildViewPage(viewPage);
		httpServer->send(200, "text/html", viewPage);
	});
	
	httpServer->begin();
	
	boardViewWebSocketServer->onEvent(boardViewEvent);	
	boardViewWebSocketServer->begin(); 
}

void BoardView::enabledRedirect(Stream &s) {
	boardViewRedirectStream=&s;
}

void BoardView::addLabel(char *name) {
	if (widgets == NULL) widgets = new Widget* [maxWidgets];
	if(widgetsCpt<maxWidgets) 
		widgets[widgetsCpt++]=new Label(this, name);
}
	
void BoardView::addEntry(char *name) {
	if (widgets == NULL) widgets = new Widget* [maxWidgets];
	if(widgetsCpt<maxWidgets) 
		widgets[widgetsCpt++]=new Entry(this, name);
}	
void BoardView::addButton(char *name, char *cmd) {
	if (widgets == NULL) widgets = new Widget* [maxWidgets];
	if(widgetsCpt<maxWidgets) 
		widgets[widgetsCpt++]=new Button(this, name, cmd);
}

void BoardView::addCheckBox(char *name, char *cmdWhenUnckeck, char *cmdWhenCheck) {
	if (widgets == NULL) widgets = new Widget* [maxWidgets];
	if(widgetsCpt<maxWidgets) 
		widgets[widgetsCpt++]=new CheckBox(this, name, cmdWhenUnckeck, cmdWhenCheck);
}

