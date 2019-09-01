#ifndef BOARD_VIEW_H
#define BOARD_VIEW_H

#define BOARD_VIEW_VERSION "0.1"

#include <BoardViewConfig.h>


#if defined(ARDUINO_ARCH_ESP32)

#include <WiFi.h>
#include <WebServer.h>
#define TcpClient WiFiClient
#define TcpServer WiFiServer

#elif defined(ARDUINO_ARCH_ESP8266)

#include <ESP8266WiFi.h>
#define TcpClient WiFiClient
#define TcpServer WiFiServer

#elif defined(ARDUINO_AVR_UNO_WIFI_REV2)

#include <WiFiNINA.h>
#define TcpClient WiFiClient
#define TcpServer WiFiServer

#else // UNO W5100

#include <Ethernet.h>
#define TcpClient EthernetClient
#define TcpServer EthernetServer

#endif

#ifdef WEB_SOCKET_SERVER
#include <WebSocketsServer.h>
#endif


#define WITH_NET // Before include BoardViewProto.h

#include <BoardViewProto.h>

//#define BOARD_VIEW_DEBUG 

typedef void (*WebSocketServerEvent)(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

class BoardView;

struct MiniWebServer {
		TcpServer *server;
		BoardView *boardView;
		unsigned port;
	    MiniWebServer(unsigned port, BoardView *boardView);
	    ~MiniWebServer();
		void loop();
};

/*
 * BoardView : multi servers manager.
 * 
 *   * WebSocket + HTTP
 *   * (TCP)
 *   * (UDP)
 *   * (MULTICAST UDP)
 * 
 *   Client part in BoardViewProto.h
 * 
 * */
 
class BoardView {
	
	public:
	
	int (*parseRequest) (char *request, char *response, unsigned len);
	unsigned maxLineLen;
	
	char *name;
	
	
	unsigned tcpPort;
	
	
	private :
	
	#if defined(WEB_SOCKET_SERVER) && defined(HTTP_SERVER)

	
	// widgets internal class
	
	struct Widget {
		String name;
		BoardView *parent;
		Widget(BoardView *parent, String name) : parent(parent), name(name) { }
		virtual String toHtml()=0;
	};
	
	struct Label : public Widget {
		Label(BoardView *parent, String name) : Widget(parent, name) {}		
		String toHtml() {
			return "<td style='font-size:"+String(parent->fontSize)+"rem'>"+
				(String)name+"</td><td>"+"<input id=\""+name+
				"\" type=\"text\" style='font-size:"+
				String(parent->fontSize)+"rem; width:70%' readonly ></td>";
		}
	};
	
	struct Entry : public Widget {
		Entry(BoardView *parent, String name) : Widget(parent, name) {}
		String toHtml() {
			return "<td style='font-size:"+String(parent->fontSize)+
				"rem'>"+(String)name+"</td><td style='font-size:"+
				String(parent->fontSize)+"%'>"+"<input id=\""+name+
				"\" type=\"text\" style='width:70%; font-size:"+
				String(parent->fontSize)+
				"rem' onClick=\"promptForInput('"+name+"');\" readonly></td>";
		}	
	};
	
	struct Button : public Widget {
		String cmd;
		Button(BoardView *parent, String name, String cmd) : Widget(parent, name), cmd(cmd) {}
		String toHtml() {
			return "<td colspan=2><input type=\"button\" style='font-size:"+
			String(parent->fontSize)+"rem' onclick=\"buttonPressed('"+cmd+"');\" value='"+name+"'></td>";
		}	
	};
	
	struct CheckBox : public Widget {
		String cmdWhenUncheck, cmdWhenCheck;
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
	
	unsigned maxWidgets;
	unsigned viewRefreshPeriodMs;
	unsigned maxHistoryLines;
	float fontSize;

	unsigned httpPort;
	unsigned webSocketPort;
		
    MiniWebServer *httpServer;
    
	#endif
		
	public :
	
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
	
	void printWithHeader(TcpClient &client, String &s);
	void urlHook(TcpClient &, char*);

	void addLabel(char *name);
	void addEntry(char *name);
	void addButton(char *name, char *cmd);
	void addCheckBox(char *name, char *cmdWhenUnckeck, char *cmdWhenCheck);
}; 

#endif


