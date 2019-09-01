#include <BoardView.h>

// source originelle : 
// https://www.hackster.io/brzi/nodemcu-websockets-tutorial-3a2013


static int (*boardViewParseRequest) (char *request, char *response, unsigned len);

static WebSocketsServer *boardViewWebSocketServer;
static Stream *boardViewRedirectStream;
static unsigned boardViewMaxLineLen;

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
			 		
			// skip leading ' '
			char *p=(char *)payload;
			while(*p == ' ' && p<(char *)payload+length) ++p;
			boardViewParseRequest((char *)p, response, boardViewMaxLineLen-1 );
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

#if defined(WEB_SOCKET_SERVER) && defined(HTTP_SERVER)

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
		  var cmds;  \n\
		  if(w) { \n\
				 \n\
				if(w.checked) { \n\
					cmds=cmdWhenCkeck \n\
				} \n\
				else { \n\
					cmds=cmdWhenUnckeck; \n\
				} \n\
				cmds.split(';').forEach(function(e) {  \n\
					Socket.send(e);  \n\
				}); \n\
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
		  Socket=new WebSocket('ws://' + window.location.hostname + ':"+String(webSocketPort)+"/'); \n\
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
		Socket=new WebSocket('ws://' + window.location.hostname + ':"+String(webSocketPort)+"/'); \n\
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
#endif
		  
BoardView::BoardView() {
	
	name="noname";
	maxLineLen=200;
	parseRequest=NULL;
	boardViewRedirectStream=NULL;
	
	tcpPort=8082;
	
	#if defined(WEB_SOCKET_SERVER) && defined(HTTP_SERVER)
	viewRefreshPeriodMs=200;
	maxWidgets=32;
	widgets=NULL;	
	widgetsCpt=0;
	maxHistoryLines=200;
	fontSize=2.0;
	httpPort=80;
	webSocketPort=8081;	
	maxHistoryLines=200;
	fontSize=2.0;	
	#endif
	
}

BoardView::~BoardView() {
	#if defined(WEB_SOCKET_SERVER) && defined(HTTP_SERVER)
	delete boardViewWebSocketServer;
	delete httpServer;
	for (int i=0; i<widgetsCpt; ++i) delete widgets[i];
	delete [] widgets;
	#endif
}

void BoardView::loop() { 
	#if defined(WEB_SOCKET_SERVER) && defined(HTTP_SERVER)
	boardViewWebSocketServer->loop(); 
	//httpServer->handleClient();
	httpServer->loop();	
	#endif
}

void BoardView::printWithHeader(TcpClient &client, String &s) {
	client.println("HTTP/1.1 200 OK");
	client.println("Content-type:text/html");
	client.println();
	client.println(s);
}

void BoardView::urlHook(TcpClient &client, char *url) {
	if(!strcmp(url, "/index.html") || !strcmp(url, "/")) {
		buildCommon();
		buildMainPage(mainPage);
		printWithHeader(client, mainPage);		
	}
	
	else if (!strcmp(url, "/console.html")) {
		buildCommon();
		buildConsolePage(consolePage);
		printWithHeader(client, consolePage);		
	}
	
	else if (!strcmp(url, "/view.html")) {
		buildCommon();
		buildViewPage(viewPage);
		printWithHeader(client, viewPage);		
	}
	
	else {
		client.println("HTTP/1.1 404 Not found !");
		client.println("Content-type:text/html");
		client.println();
		client.println("<html><body><h1>Not found !</h1></body></html>");
		client.println();
	}
}


void BoardView::begin() { 
	boardViewMaxLineLen=maxLineLen;
	boardViewParseRequest=parseRequest;
	
	#if defined(WEB_SOCKET_SERVER) && defined(HTTP_SERVER)	
	boardViewWebSocketServer = new WebSocketsServer(webSocketPort);
	
	httpServer = new MiniWebServer(httpPort, this);
	
	Serial.println("MiniWebServer started.");
	
	boardViewWebSocketServer->onEvent(boardViewEvent);	
	boardViewWebSocketServer->begin(); 
	#endif
}

void BoardView::enabledRedirect(Stream &s) {
	boardViewRedirectStream=&s;
}

#if defined(WEB_SOCKET_SERVER) && defined(HTTP_SERVER)

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


// MiniWebServer 

MiniWebServer::MiniWebServer(unsigned port, BoardView *boardView) : port(port), boardView(boardView) {
	server=new TcpServer(port); 
	server->begin();
}

MiniWebServer::~MiniWebServer() {
	delete server;
}


void MiniWebServer::loop() {

	TcpClient client;
	char line[60];
	char url[20];
	int n;
	
	while(client=server->available()) {
		
		// lecture de la requête
		
		while(n=client.readBytesUntil('\n', line, 60)) {
			line[n]=0;
			
			if(!strncmp(line, "GET", 3)) {
				char *s=line+4;
				char *e=s;
				while(*e != ' ' && *e) ++e;
				*e=0; 
				strcpy(url, s);
			}
				 			
			if(strlen(line)<3) break;
		}
		
		// send response

		boardView->urlHook(client, url);	
	}	
	client.stop();
			
}

#endif
