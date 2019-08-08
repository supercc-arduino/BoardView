#ifndef BOARD_VIEW_H
#define BOARD_VIEW_H

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>

#include <BoardViewProto.h>


extern WebSocketsServer *boardViewWebSocketServer;
extern int (*boardViewParseRequest) (char *request, char *response, unsigned len);
extern Stream *boardViewRedirectStream;
extern unsigned boardViewMaxLineLen;

typedef void (*WebSocketServerEvent)(uint8_t num, WStype_t type, uint8_t * payload, size_t length);


/*
 * Unique classe définie par le module.
 * gère un serveur http et un serveur webSocket
 *
 * */

class BoardView {
	
	private :
	
	// widgets internal class
	
	class Widget {
		protected :
		
		String name;
		BoardView *parent;
		
		public:
		
		Widget(BoardView *parent, String name) : parent(parent), name(name) { }
		virtual String toHtml()=0;
	};
	
	class Label : public Widget {
		public :
		Label(BoardView *parent, String name) : Widget(parent, name) {}		
		String toHtml() {
			return "<td style='font-size:"+String(parent->fontSize)+"rem'>"+(String)name+"</td><td>"+"<input id=\""+name+"\" type=\"text\" style='font-size:"+String(parent->fontSize)+"rem; width:70%' readonly ></td>";
		}
	};
	
	class Entry : public Widget {
		
		public :
		
		Entry(BoardView *parent, String name) : Widget(parent, name) {}
		
		String toHtml() {
			return "<td style='font-size:"+String(parent->fontSize)+"rem'>"+(String)name+"</td><td style='font-size:"+String(parent->fontSize)+"%'>"+"<input id=\""+name+"\" type=\"text\" style='width:70%; font-size:"+String(parent->fontSize)+"rem' onClick=\"promptForInput('"+name+"');\" readonly></td>";
		}	
	};
	
	class Button : public Widget {
		
		String cmd;
		
		public :
		
		Button(BoardView *parent, String name, String cmd) : Widget(parent, name), cmd(cmd) {}
		
		String toHtml() {
			return "<td colspan=2><input type=\"button\" style='font-size:"+
			String(parent->fontSize)+"rem' onclick=\"buttonPressed('"+cmd+"');\" value='"+name+"'></td>";
		}	
	};
	
	class CheckBox : public Widget {
		
		String cmdWhenUncheck, cmdWhenCheck;
		
		public :
		
		CheckBox(BoardView *parent, String name, String cmdWhenUncheck, String cmdWhenCheck) : 
			Widget(parent, name), cmdWhenUncheck(cmdWhenUncheck), cmdWhenCheck(cmdWhenCheck) {}
		
		String toHtml() {
			return "\n<td><label for='"+name+"' style='font-size:"+
				String(parent->fontSize)+"rem'>"+name+"</label></td>" \
				"<td><input id='"+name+"' type='checkbox' style='width:50px; height:50px;' \
				onclick=\"checkPressed('"+name+"', '"+cmdWhenUncheck+"', '"+cmdWhenCheck+"');\"></td>";
		}	
	};
	
	unsigned widgetsCpt;
	BoardView::Widget **widgets;
	
	public :
	
	String title;
	String header;
	String footer;
	String styles;
	String javaScriptCommon;
	String mainPage;
	String consolePage;
	String viewPage;
	
	char *name;
	unsigned maxWidgets;
	unsigned viewRefreshPeriodMs;
	unsigned maxHistoryLines;
	float fontSize;

	unsigned httpPort, webSocketPort, tcpPort;
    ESP8266WebServer *httpServer;
	BoardView();

	~BoardView();

	void loop();
	void begin();

	void buildCommon();
	void buildMainPage(String &);
	void buildConsolePage(String &s);
	void buildViewPage(String &);
	
	void onEvent(WebSocketServerEvent cbEvent);
	void enabledRedirect(Stream &s);

	void addLabel(char *name);
	void addEntry(char *name);
	void addButton(char *name, char *cmd);
	void addCheckBox(char *name, char *cmdWhenUnckeck, char *cmdWhenCheck);
}; 

#endif

