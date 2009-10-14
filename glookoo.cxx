#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <gloox/client.h>
#include <gloox/connectionlistener.h>
#include <gloox/messagesessionhandler.h>
#include <gloox/messageeventhandler.h>
#include <gloox/messagehandler.h>
#ifdef HAVE_GLOOX_MESSAGE_H
#include <gloox/message.h>
#endif
#include <gloox/messagesession.h>
#include <stdlib.h>
#include <pthread.h>
#include <readline/readline.h>

#ifdef _WIN32
#include <windows.h>
#define strcasecmp(x, y) stricmp(x, y)
#endif

static char* str_to_utf8_alloc(const char* str) {
#ifdef _WIN32
	size_t in_len = strlen(str);
	wchar_t* wcsdata;
	char* mbsdata;
	size_t mbssize, wcssize;

	wcssize = MultiByteToWideChar(GetACP(), 0, str, in_len,  NULL, 0);
	wcsdata = (wchar_t*) malloc((wcssize + 1) * sizeof(wchar_t));
	wcssize = MultiByteToWideChar(GetACP(), 0, str, in_len, wcsdata, wcssize + 1);
	wcsdata[wcssize] = 0;

	mbssize = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR) wcsdata, -1, NULL, 0, NULL, NULL);
	mbsdata = (char*) malloc((mbssize + 1));
	mbssize = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR) wcsdata, -1, mbsdata, mbssize, NULL, NULL);
	mbsdata[mbssize] = 0;
	free(wcsdata);
	return mbsdata;
#else
	return strdup(str);
#endif
}

static char* utf8_to_str_alloc(const char* utf8) {
#ifdef _WIN32
	size_t in_len = strlen(utf8);
	wchar_t* wcsdata;
	char* mbsdata;
	size_t mbssize, wcssize;

	wcssize = MultiByteToWideChar(CP_UTF8, 0, utf8, in_len,  NULL, 0);
	wcsdata = (wchar_t*) malloc((wcssize + 1) * sizeof(wchar_t));
	wcssize = MultiByteToWideChar(CP_UTF8, 0, utf8, in_len, wcsdata, wcssize + 1);
	wcsdata[wcssize] = 0;

	mbssize = WideCharToMultiByte(GetACP(), 0, (LPCWSTR) wcsdata, -1, NULL, 0, NULL, NULL);
	mbsdata = (char*) malloc((mbssize + 1));
	mbssize = WideCharToMultiByte(GetACP(), 0, (LPCWSTR) wcsdata, -1, mbsdata, mbssize, NULL, NULL);
	mbsdata[mbssize] = 0;
	free(wcsdata);
	return mbsdata;
#else
	return strdup(utf8);
#endif
}

class Glookoo : public gloox::ConnectionListener, gloox::MessageHandler
{
public:
	Glookoo(const char* server, const char* jid, const char* passwd, const char* user)
			: server_(server), jid_(jid), passwd_(passwd), user_(user) {
		client_ = NULL;
		session_ = NULL;
	}

	virtual ~Glookoo() {
		//if (client_) delete client_;
	}

	virtual void onConnect() {
		displayMessage("=== connected ===");
		session_ = new gloox::MessageSession(client_, gloox::JID(user_));
		session_->registerMessageHandler(this);
	}

#ifdef HAVE_GLOOX_MESSAGE_H
	virtual void handleMessage(const gloox::Message& msg, gloox::MessageSession *session) {
		char* message = utf8_to_str_alloc(msg.body().c_str());
		displayMessage(message);
		free(message);
	}
#else
	virtual void handleMessage(gloox::Stanza *stanza, gloox::MessageSession *session) {
		char* message = utf8_to_str_alloc(stanza->body().c_str());
		displayMessage(message);
		free(message);
	}
#endif

	virtual void onDisconnect(gloox::ConnectionError reason) {
		displayMessage("=== disconnected ===");
		if (session_) {
			delete session_;
			session_ = NULL;
		}
	}

	virtual bool onTLSConnect(const gloox::CertInfo &info) {
		return true;
	}

	void run() {
		client_ = new gloox::Client(gloox::JID(jid_), passwd_);
		client_->registerConnectionListener(this);
		client_->setServer(server_);
		client_->connect();
	}

	void stop() {
		client_->disconnect();
	}

	void sendMessage(const char* message) {
		if (session_) {
			char* send_message = str_to_utf8_alloc(message);
			session_->send(send_message, "");
			free(send_message);
		}
	}
	
	void displayMessage(const char* message) {
		int point = rl_point;
		rl_beg_of_line(0, 0);
		rl_redisplay();
		printf("%s\n", message);
		rl_point = point;
		rl_refresh_line(0, 0);
	}

private:
	gloox::Client* client_;
	gloox::MessageSession* session_;
	const char* server_;
	const char* jid_;
	const char* passwd_;
	const char* user_;
};

static Glookoo* glookoo = NULL;

static void* input_func(void* arg) {
	pthread_detach(pthread_self());
	while (glookoo) {
		rl_callback_read_char();
	}
	return 0;
}

static void* gloox_func(void* arg) {
	glookoo->run();
}

static void input_handler(void) {
	char* message = rl_copy_text(0, rl_end);
	if (glookoo && strlen(message)) {
		rl_beg_of_line(0, 0);
		rl_redisplay();
		if (*message != '/')
			glookoo->sendMessage(message);
		else {
			if (!strcasecmp(message + 1, "quit")) glookoo->stop();
		}
	}
	free(message);
}

int  opterr = 1;
int  optind = 1;
int  optopt;
char *optarg;
 
static int getopt(int argc, char** argv, const char* opts) {
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1) {
		if(optind >= argc ||
				argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == 0) {
			optind++;
			return(EOF);
		}
	}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

int main(int argc, char* argv[]) {
	pthread_t pt_input, pt_gloox;
	char* config = NULL;
	const char* server = "talk.google.com";
	char* jid = NULL;
	char* passwd = NULL;
	const char* user = "wassr-bot@wassr.jp";
	int c;

	opterr = 0;
	while ((c = getopt(argc, (char**)argv, "c:j:p:s:u:") != -1)) {
		switch (optopt) {
		case 'c': config = optarg; break;
		case 'j': jid = optarg; break;
		case 'p': passwd = optarg; break;
		case 's': server = optarg; break;
		case 'u': user = optarg; break;
		case '?': break;
		default:
			argc = 0;
			break;
		}
		optarg = NULL;
	}
	
	if (config) {
		char buf[BUFSIZ];
		FILE *fp = fopen(config, "r");
		if (!fp) {
			fprintf (stderr, "Usage: glookoo [-c config-file] [-s server] [-j jid] [-p passwd] [-u user]\n");
			return -1;
		}
		while(fgets(buf, sizeof(buf), fp)) {
			char* line = buf;
			char* ptr = strpbrk(line, "\r\n");
			if (ptr) *ptr = 0;
			if (!strncmp(line, "server=", 7)) server = strdup(line+7);
			if (!strncmp(line, "jid=", 4)) jid = strdup(line+4);
			if (!strncmp(line, "passwd=", 7)) passwd = strdup(line+7);
			if (!strncmp(line, "user=", 5)) user = strdup(line+5);
		}
		fclose(fp);
	}

	if (!jid || !passwd) {
		fprintf (stderr, "Usage: glookoo [-c config-file] [-s server] [-j jid] [-p passwd] [-u user]\n");
		exit (1);
	}
	
	rl_callback_handler_install("#> ", (void(*)(char*))input_handler);

	Glookoo* tmp = glookoo = new Glookoo(server, jid, passwd, user);

	pthread_create(&pt_input, NULL, &input_func, glookoo);

	glookoo->run();
	glookoo = NULL;
	delete tmp;

	pthread_join(pt_input, NULL);

	rl_deprep_terminal();
	return 0;
}
