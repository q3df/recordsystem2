#include <stdio.h>
#include "Plugin.h"
#include "../Recordsystem.h"
#include "../Logger.h"

#include <libxml/encoding.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

namespace {
	xmlAttr *getXmlAttribute(xmlAttr *attr, const char *name) {
		xmlAttr *curAttr = NULL;
		for (curAttr = attr; curAttr; curAttr = curAttr->next) {
			if(!strcmp((const char *)curAttr->name, name))
				return curAttr;
		}

		return NULL;
	}
	
	char* my_itoa(int val, int base) {
		static char buf[32] = {0};
		int i = 30;

		memset(&buf, 0, sizeof(buf));

		for(; val && i ; --i, val /= base)
			buf[i] = "0123456789abcdef"[val % base];

		return &buf[i+1];
	}

	void refreshPlayerDemoHistory(const char *fileName, const char *demofilename, int mstime) {
		static char ret[8096];
		static char ret2[8096];
		char buffer[8096];
		int oldMsTime = 0;
		int itemCount = 0;

		xmlDocPtr doc;
		xmlNode *rootElement = NULL;
		xmlNode *curNode = NULL;
		xmlAttr *curAttr = NULL;
		xmlTextWriterPtr writer;
		xmlBufferPtr buf;

		memset(&ret, 0, sizeof(ret));
		memset(&ret2, 0, sizeof(ret2));
		memset(&buffer, 0, sizeof(buffer));

		buf = xmlBufferCreate();
		if(buf == NULL) {
			gRecordsystem->GetSyscalls()->Print("cant create xml buffer to write demo history...\n");
			return;
		}

		writer = xmlNewTextWriterMemory(buf, 0);
		if(writer == NULL) {
			gRecordsystem->GetSyscalls()->Print("cant create xml text writer to write demo history...");
			return;
		}

		xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
		xmlTextWriterStartElement(writer, BAD_CAST "History");

		doc = xmlReadFile(fileName, NULL, 0);

		if(doc != NULL) {
			rootElement = xmlDocGetRootElement(doc);

			curAttr = getXmlAttribute(rootElement->properties, "count");
			if(curAttr != NULL) {
				itemCount = atoi((const char *)curAttr->children->content);

				for (curNode = rootElement->children; curNode; curNode = curNode->next) {
					if(curNode->type == XML_ELEMENT_NODE) {
						curAttr = getXmlAttribute(curNode->properties, "mstime");
						if(curAttr != NULL && atoi((const char *)curAttr->children->content) > oldMsTime) {
							oldMsTime = atoi((const char *)curAttr->children->content);
							curAttr = getXmlAttribute(curNode->properties, "filename");
							if(curAttr != NULL)
								strncpy(ret, (const char *)curAttr->children->content, sizeof(ret));
						}
					}
				}

				if(itemCount < 2)
					xmlTextWriterWriteAttribute(writer, BAD_CAST "count", BAD_CAST my_itoa(itemCount+1, 10));
				else
					xmlTextWriterWriteAttribute(writer, BAD_CAST "count", BAD_CAST my_itoa(itemCount, 10));

				xmlTextWriterStartElement(writer, BAD_CAST "Item");
				xmlTextWriterWriteAttribute(writer, BAD_CAST "filename", BAD_CAST demofilename);
				xmlTextWriterWriteAttribute(writer, BAD_CAST "mstime", BAD_CAST my_itoa(mstime, 10));
				xmlTextWriterEndElement(writer);

				for (curNode = rootElement->children; curNode; curNode = curNode->next) {
					if(curNode->type == XML_ELEMENT_NODE) {
						curAttr = getXmlAttribute(curNode->properties, "mstime");
						if((curAttr != NULL && atoi((const char *)curAttr->children->content) != oldMsTime) || itemCount < 2) {
							curAttr = getXmlAttribute(curNode->properties, "filename");
							if(curAttr != NULL) {
								xmlTextWriterStartElement(writer, BAD_CAST "Item");
								xmlTextWriterWriteAttribute(writer, BAD_CAST "filename", curAttr->children->content);
								xmlTextWriterWriteAttribute(writer, BAD_CAST "mstime", curNode->properties->next->children->content);
								xmlTextWriterEndElement(writer);
							}
						}
					}
				}
			}
		}else{
			xmlTextWriterWriteAttribute(writer, BAD_CAST "count", BAD_CAST my_itoa(itemCount+1, 10));

			xmlTextWriterStartElement(writer, BAD_CAST "Item");
			xmlTextWriterWriteAttribute(writer, BAD_CAST "filename", BAD_CAST demofilename);
			xmlTextWriterWriteAttribute(writer, BAD_CAST "mstime", BAD_CAST my_itoa(mstime, 10));
			xmlTextWriterEndElement(writer);
		}

		xmlTextWriterEndElement(writer);
		xmlTextWriterEndDocument(writer);
		xmlFreeTextWriter(writer);

		FILE *fp = fopen(fileName, "w+");
		if(fp != NULL) {
			fputs((const char *)buf->content, fp);
			fclose(fp);
		}

		if(itemCount > 1) {
			unlink(ret);
	#ifndef WIN32
			snprintf(ret2, sizeof(ret2), "%s.gz", ret);
			unlink(ret2);
	#endif
		}

		xmlFreeDoc(doc);
		xmlBufferFree(buf);
		xmlCleanupParser();
	}
}

class ServerDemosPlugin : public PluginBase {
public:
	virtual void Init() {
	}

	virtual void Destroy() {
	}
};

// Register plugin!
static PluginProxy<ServerDemosPlugin> gServerDemosPlugin;
