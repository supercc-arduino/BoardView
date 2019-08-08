#include <BoardViewProto.h>

char *f2s(char *s, int len, float f, int prec) {	
	if(prec>8) prec=8;
	
	dtostrf(f, -(len-1), prec, s);
	char *p=s+strlen(s)-1;
	while(*p == ' ') { *p=0; p=p-1; }
	return s;
} 


float s2f(char *s) {
	return atof(s);
}


char *addInt(char *dst, int len, char *name, int value) {
	char tmp[20];
	
	if(dst[0]!= 0)
		sprintf(tmp, "; %s=%d", name, value);
	else
		sprintf(tmp, "%s=%d", name, value);
			
	if(strlen(dst)+strlen(tmp)<len-1) strcat(dst, tmp);
		
	return dst;
}


char *addFloat(char *dst, int len, char *name, float value, int prec) {		
	char tmp[20];
	
	if(dst[0] != 0)
		sprintf(tmp, "; %s=", name);
	else
		sprintf(tmp, "%s=", name);
		
	char tmpFloat[12]=" ";
	
	f2s(tmpFloat, 12, value, prec);
	if(strlen(dst)+strlen(tmp)+strlen(tmpFloat)<len-1) {
		strcat(dst, tmp); strcat(dst, tmpFloat);
	} 
	
	return dst;
}


char *addStr(char *dst, int len, char *name, char *s) {
	char tmp[len];
	
	if(dst[0] != 0)
		sprintf(tmp, "; %s=\"%s\"", name, s);
	else
		sprintf(tmp, "%s=\"%s\"", name, s);
		
	if(strlen(dst)+strlen(tmp)<len-1) strcat(dst, tmp);	
}


int matchCmd(char *line, char *cmd) {
	int cmdLen=strlen(cmd);
	if(!strncmp(line,cmd,cmdLen)) 
		if(line[cmdLen]==' ' || !line[cmdLen])
			return 1;
	return 0;
}


int matchAndAssignFloat(char *line, char *varName, float *varAddr) {
	int varLen=strlen(varName);
	if(!strncmp(line, varName, varLen)) 
		if(line[varLen]==' ' || line[varLen]=='=') {
			*varAddr=atof(&line[varLen+1]);
			return 1;
		}
	return 0;
}


int matchAndAssignInt(char *line, char *varName, int *varAddr) {
	int varLen=strlen(varName);
	if(!strncmp(line, varName, varLen)) 
		if(line[varLen]==' ' || line[varLen]=='=') {
			*varAddr=atoi(&line[varLen+1]);
			return 1;
		}
	return 0;
}


int matchAndAssignStr(char *line, char *varName, char *varAddr, int len) {
	int varLen=strlen(varName);
	if(!strncmp(line, varName, varLen)) 
		if(line[varLen]==' ' || line[varLen]=='=') {
			
			// remove " around str
			int lineLen=strlen(line);
			char *start=&line[varLen+1];
			if(*start=='"' && line[lineLen-1]=='"' ) {
				++start; 
			}
			
			strncpy(varAddr, start, len);
			varAddr[len-1]=0;
			int varLen=strlen(varAddr);
			if(varAddr[varLen-1]=='"') varAddr[varLen-1]=0;

			return 1;
		}
	return 0;
}


int readAndParseLines(Stream &stream, int (*parseRequest)(char *request, char *response, int len), int len) {
	while (stream.available() > 0) {
		char line[len];
		char response[len];
		int n;
				
		n=stream.readBytesUntil('\n', line, len);
		if(n) {
			line[n]=0; 
			
			// remove trailing '\r\n';
			
			int e=strlen(line)-1; 
			if(line[e]=='\r') { line[e--]=0; }
			if(line[e]=='\n') line[e]=0; 
			
			parseRequest(line, response, len );
			stream.println(response);
			stream.flush();
		}
	}	
	return 0;
}

