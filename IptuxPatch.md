#我针对iptux-0.5.1-2做的补丁，功能如下:
#1）可选弹出置顶提示按钮（类似飞鸽的弹出）
#2) 去除收到群发信息后，可能误发群信息的不当设置
#3）默认以组的方式显示列表（局域网多组的很好用了）
下面的显示不正常，请参考http://www.linuxsir.org/bbs/showthread.php?p=2130879#post2130879
#iptux-0.5.1.patch

--- src/UdpData.cpp	2011-03-12 10:38:46.000000000 +0800
+++ src/UdpData.cpp	2011-03-13 10:27:44.000000000 +0800
@@ -311,15 +311,15 @@
> text = ipmsg\_get\_attach(buf, ':', 5);
> if (text && **text != '\0') {
> > /**/**插入消息**/
-		if ((commandno & IPMSG\_BROADCASTOPT) || (commandno & IPMSG\_MULTICASTOPT))
-			InsertMessage(pal, BROADCAST\_TYPE, text);
-		else
+		//if ((commandno & IPMSG\_BROADCASTOPT) || (commandno & IPMSG\_MULTICASTOPT))
+		//	InsertMessage(pal, BROADCAST\_TYPE, text);
+		//else
> > > InsertMessage(pal, REGULAR\_TYPE, text);

> > /**/** 注册消息 **/
> > pthread\_mutex\_lock(cthrd.GetMutex());
-		if ((commandno & IPMSG\_BROADCASTOPT) || (commandno & IPMSG\_MULTICASTOPT))
-			grpinf = cthrd.GetPalBroadcastItem(pal);
-		else
+		//if ((commandno & IPMSG\_BROADCASTOPT) || (commandno & IPMSG\_MULTICASTOPT))
+		//	grpinf = cthrd.GetPalBroadcastItem(pal);
+		//else
> > > grpinf = cthrd.GetPalRegularItem(pal);

> > if (!grpinf->dialog && !cthrd.MsglineContainItem(grpinf))
> > > cthrd.PushItemToMsgline(grpinf);
--- src/DataSettings.cpp	2009-11-20 17:12:15.000000000 +0800
+++ src/DataSettings.cpp	2009-12-24 23:47:18.000000000 +0800
@@ -305,6 +305,11 @@

> gtk\_box\_pack\_start(GTK\_BOX(hbox), widget, TRUE, TRUE, 0);
> g\_datalist\_set\_data(&widset, "font-chooser-widget", widget);**

+	/**消息到来后弹出窗口提示**/
+	widget = gtk\_check\_button\_new\_with\_label(
+			 _("Pop windows while recved message"));
+	gtk\_box\_pack\_start(GTK\_BOX(box), widget, FALSE, FALSE, 0);
+	g\_datalist\_set\_data(&widset, "popmsg-check-widget", widget);
> /**隐藏面板，只显示状态图标**/
> widget = gtk\_check\_button\_new\_with\_label(
> >_("Automatically hide the panel after login"));
@@ -546,6 +551,9 @@

> gtk\_combo\_box\_set\_active(GTK\_COMBO\_BOX(widget), active);
> widget = GTK\_WIDGET(g\_datalist\_get\_data(&widset, "font-chooser-widget"));
> gtk\_font\_button\_set\_font\_name(GTK\_FONT\_BUTTON(widget), progdt.font);
+	widget = GTK\_WIDGET(g\_datalist\_get\_data(&widset, "popmsg-check-widget"));
+	gtk\_toggle\_button\_set\_active(GTK\_TOGGLE\_BUTTON(widget),
+				 FLAG\_ISSET(progdt.flags, 7));
> widget = GTK\_WIDGET(g\_datalist\_get\_data(&widset, "statusicon-check-widget"));
> gtk\_toggle\_button\_set\_active(GTK\_TOGGLE\_BUTTON(widget),
> > FLAG\_ISSET(progdt.flags, 6));
@@ -1005,6 +1013,11 @@

> g\_free(progdt.font);
> progdt.font = g\_strdup(gtk\_font\_button\_get\_font\_name(GTK\_FONT\_BUTTON(widget)));

+	widget = GTK\_WIDGET(g\_datalist\_get\_data(&widset, "popmsg-check-widget"));
+	if (gtk\_toggle\_button\_get\_active(GTK\_TOGGLE\_BUTTON(widget)))
+		FLAG\_SET(progdt.flags, 7);
+	else
+		FLAG\_CLR(progdt.flags, 7);
> widget = GTK\_WIDGET(g\_datalist\_get\_data(&widset, "statusicon-check-widget"));
> if (gtk\_toggle\_button\_get\_active(GTK\_TOGGLE\_BUTTON(widget)))
> > FLAG\_SET(progdt.flags, 6);
--- src/MainWindow.cpp	2009-11-20 17:12:15.000000000 +0800
+++ src/MainWindow.cpp	2009-12-25 08:20:56.000000000 +0800
@@ -23,6 +23,7 @@
  1. nclude "callback.h"
  1. nclude "support.h"
  1. nclude "utils.h"
+#include "output.h"

> extern ProgramData progdt;
> extern CoreThread cthrd;
> extern MainWindow mwin;
@@ -739,7 +740,7 @@
> > gtk\_scrolled\_window\_set\_shadow\_type(GTK\_SCROLLED\_WINDOW(sw),
> > > GTK\_SHADOW\_ETCHED\_IN);

-	model = GTK\_TREE\_MODEL(g\_datalist\_get\_data(&mdlset, "regular-paltree-model"));
+	model = GTK\_TREE\_MODEL(g\_datalist\_get\_data(&mdlset, "group-paltree-model"));

> widget = CreatePaltreeTree(model);
> g\_object\_set\_data(G\_OBJECT(widget), "paltree-model", model);
> gtk\_container\_add(GTK\_CONTAINER(sw), widget);
@@ -1479,6 +1480,7 @@
> GdkColor **color;**

> if (blinking) {
+		if (progdt.flags&0x80 && IsMessageShow){IsMessageShow=TRUE;pop\_info1(NULL, _("Some one said you"));}
> > gtk\_tree\_model\_get(model, iter, 5, &color, -1);
> > if ((color->red == color1.red) && (color->green == color1.green)
> > > && (color->blue == color1.blue))
@@ -1486,6 +1488,7 @@

> > else
> > > gtk\_tree\_store\_set(GTK\_TREE\_STORE(model),iter, 5, &color1, -1);

> } else
+		IsMessageShow=FALSE;
> > gtk\_tree\_store\_set(GTK\_TREE\_STORE(model), iter, 5, &color1, -1);

> }_

--- src/MainWindow.h	2009-11-20 17:12:15.000000000 +0800
+++ src/MainWindow.h	2009-12-24 22:41:32.000000000 +0800
@@ -84,6 +84,7 @@
> GList **tmdllist;		//model链表，用于构建model循环结构
> GtkAccelGroup**accel;	//快捷键集组
> guint timerid;		//UI更新定时器ID
+	bool IsMessageShow;
> private:
> > static GtkWidget **CreateTransPopupMenu(GtkTreeModel**model);
> > static GtkWidget **CreatePaltreePopupMenu(GroupInfo**grpinf);
--- src/ProgramData.cpp	2009-11-20 17:12:15.000000000 +0800
+++ src/ProgramData.cpp	2009-12-24 23:05:52.000000000 +0800
@@ -101,6 +101,8 @@
> > gconf\_client\_set\_string(client, GCONF\_PATH "/preference\_encode", encode, NULL);
> > gconf\_client\_set\_string(client, GCONF\_PATH "/pal\_icon", palicon, NULL);
> > gconf\_client\_set\_string(client, GCONF\_PATH "/panel\_font", font, NULL);
+	gconf\_client\_set\_bool(client, GCONF\_PATH "/pop\_msg",
+			 FLAG\_ISSET(flags, 7) ? TRUE : FALSE, NULL);
> > gconf\_client\_set\_bool(client, GCONF\_PATH "/hide\_startup",
> > > FLAG\_ISSET(flags, 6) ? TRUE : FALSE, NULL);

> > gconf\_client\_set\_bool(client, GCONF\_PATH "/open\_transmission",
@@ -219,6 +221,8 @@
> > > palicon = g\_strdup("icon-qq.png");

> > if (!(font = gconf\_client\_get\_string(client, GCONF\_PATH "/panel\_font", NULL)))
> > > font = g\_strdup("Sans Serif 10");
+	if (gconf\_client\_get\_bool(client, GCONF\_PATH "/pop\_msg", NULL))
+		FLAG\_SET(flags, 7);

> > if (gconf\_client\_get\_bool(client, GCONF\_PATH "/hide\_startup", NULL))
> > > FLAG\_SET(flags, 6);

> > if (gconf\_client\_get\_bool(client, GCONF\_PATH "/open\_transmission", NULL))
@@ -495,6 +499,11 @@
> > > g\_free(progdt->font);
> > > progdt->font = g\_strdup(str);
> > > }
+	} else if (strcmp(entry->key, GCONF\_PATH "/pop\_msg") == 0) {
+		if (gconf\_value\_get\_bool(entry->value))
+			FLAG\_SET(progdt->flags, 7);
+		else
+			FLAG\_CLR(progdt->flags, 7);

> > } else if (strcmp(entry->key, GCONF\_PATH "/hide\_startup") == 0) {
> > > if (gconf\_value\_get\_bool(entry->value))
> > > > FLAG\_SET(progdt->flags, 6);
--- src/ProgramData.h	2009-11-20 17:12:15.000000000 +0800
+++ src/ProgramData.h	2009-12-24 23:42:36.000000000 +0800
@@ -49,7 +49,7 @@

> > char **encode;		//默认通信编码**
> > char **palicon;		//默认头像**
> > char **font;		//面板字体**
-	uint8\_t flags;		//6 图标,5 传输:4 enter:3 历史:2 日志:1 黑名单:0 共享
+	uint8\_t flags;		//7 消息到来弹出提示: 6 图标,5 传输:4 enter:3 历史:2 日志:1 黑名单:0 共享


> char **transtip;		//传输完成提示声音**
> char **msgtip;		//消息到来提示声音**
--- src/StatusIcon.h	2009-11-20 17:12:15.000000000 +0800
+++ src/StatusIcon.h	2009-12-24 22:55:42.000000000 +0800
@@ -21,6 +21,7 @@

> void CreateStatusIcon();
> void AlterStatusIconMode();
+	static void StatusIconActivate();
> private:
> > GtkStatusIcon **statusicon;
> > guint timerid;
@@ -30,7 +31,6 @@

> //回调处理部分
> private:
> > static void ShowTransWindow();
-	static void StatusIconActivate();
> > static void PopupWorkMenu(GtkStatusIcon**statusicon, guint button, guint time);
> > static gboolean StatusIconQueryTooltip(GtkStatusIcon **statusicon, gint x, gint y,
> > > gboolean key, GtkTooltip**tooltip);
--- src/output.cpp	2009-11-20 17:12:15.000000000 +0800
+++ src/output.cpp	2009-12-24 22:56:52.000000000 +0800
@@ -12,6 +12,41 @@
  1. nclude "output.h"
  1. nclude "sys.h"

+#include "StatusIcon.h"
+extern StatusIcon sicon;
+/
+ **无模式弹出消息提示.
+** @param parent parent window
+ **@param format as in printf()
+** @param ...
+ **/
+void destroy( GtkWidget**widget, GtkWidget **parent ) {
+	sicon.StatusIconActivate();
+	gtk\_widget\_destroy(parent);
+	gtk\_widget\_destroy(widget);
+}
+void pop\_info1(GtkWidget**parent, const gchar **format, ...)
+{
+	GtkWidget**dialog,**button;
+	gchar**msg;
+	va\_list ap;
+
+	va\_start(ap, format);
+	msg = g\_strdup\_vprintf(format, ap);
+	va\_end(ap);
+	dialog = gtk\_dialog\_new();
+	button = gtk\_button\_new\_with\_label(msg);
+	g\_free(msg);
+	gtk\_signal\_connect (GTK\_OBJECT (button), "clicked", GTK\_SIGNAL\_FUNC (destroy),dialog);
+	gtk\_container\_add (GTK\_CONTAINER (GTK\_DIALOG(dialog)->vbox), button);
+	gtk\_widget\_show (button);
+
+	gtk\_widget\_show\_all(dialog);
+	gtk\_window\_set\_keep\_above(GTK\_WINDOW(dialog),TRUE);
+//	gtk\_dialog\_run(GTK\_DIALOG(dialog));
+//	gtk\_widget\_destroy(dialog);
+}
+

> /
    * 弹出消息提示.
    * @param parent parent window
--- src/output.h	2009-11-20 17:12:15.000000000 +0800
+++ src/output.h	2009-12-24 21:00:02.000000000 +0800
@@ -35,6 +35,7 @@
  1. efine ptrace(format,...) printf(format,##VA\_ARGS)
  1. ndif

+void pop\_info1(GtkWidget **parent, const gchar**format, ...);
> void pop\_info(GtkWidget **parent, const gchar**format, ...);
> void pop\_warning(GtkWidget **parent, const gchar**format, ...);
> void pop\_error(const gchar **format, ...);
--- src/patch	1970-01-01 08:00:00.000000000 +0800
+++ src/patch	2010-09-15 07:55:50.000000000 +0800
@@ -0,0 +1,111 @@
+DataSettings.cpp
+308,312d307
+< 	/** 消息到来后弹出窗口提示 **/
+< 	widget = gtk\_check\_button\_new\_with\_label(
+< 			 _("Pop windows while recved message"));
+< 	gtk\_box\_pack\_start(GTK\_BOX(box), widget, FALSE, FALSE, 0);
+< 	g\_datalist\_set\_data(&widset, "popmsg-check-widget", widget);
+554,556d548
+< 	widget = GTK\_WIDGET(g\_datalist\_get\_data(&widset, "popmsg-check-widget"));
+< 	gtk\_toggle\_button\_set\_active(GTK\_TOGGLE\_BUTTON(widget),
+< 				 FLAG\_ISSET(progdt.flags, 7));
+1016,1020d1007
+< 	widget = GTK\_WIDGET(g\_datalist\_get\_data(&widset, "popmsg-check-widget"));
+< 	if (gtk\_toggle\_button\_get\_active(GTK\_TOGGLE\_BUTTON(widget)))
+< 		FLAG\_SET(progdt.flags, 7);
+< 	else
+< 		FLAG\_CLR(progdt.flags, 7);
+
+
+
+MainWindow.cpp
+26d25
+< #include "output.h"
+743c742
+< 	model = GTK\_TREE\_MODEL(g\_datalist\_get\_data(&mdlset, "group-paltree-model"));
+---
+> 	model = GTK\_TREE\_MODEL(g\_datalist\_get\_data(&mdlset, "regular-paltree-model"));
+1483d1481
+< 		if (progdt.flags&0x80 && IsMessageShow){IsMessageShow=TRUE;pop\_info1(NULL,_("Some one said you"));}
+1491d1488
+< 		IsMessageShow=FALSE;
+
+
+
+ProgramData.cpp
+104,105d103
+< 	gconf\_client\_set\_bool(client, GCONF\_PATH "/pop\_msg",
+< 			 FLAG\_ISSET(flags, 7) ? TRUE : FALSE, NULL);
+224,225d221
+< 	if (gconf\_client\_get\_bool(client, GCONF\_PATH "/pop\_msg", NULL))
+< 		FLAG\_SET(flags, 7);
+502,506d497
+< 	} else if (strcmp(entry->key, GCONF\_PATH "/pop\_msg") == 0) {
+< 		if (gconf\_value\_get\_bool(entry->value))
+< 			FLAG\_SET(progdt->flags, 7);
+< 		else
+< 			FLAG\_CLR(progdt->flags, 7);
+
+
+
+
+output.cpp
+15,48d14
+< #include "StatusIcon.h"
+< extern StatusIcon sicon;
+< /****+<** 无模式弹出消息提示.
+<  **@param parent parent window
+<** @param format as in printf()
+<  **@param ...
+<**/
+< void destroy( GtkWidget **widget, GtkWidget**parent ) {
+< 	sicon.StatusIconActivate();
+< 	gtk\_widget\_destroy(parent);
+< 	gtk\_widget\_destroy(widget);
+< }
+< void pop\_info1(GtkWidget **parent, const gchar**format, ...)
+< {
+< 	GtkWidget **dialog,**button;
+< 	gchar **msg;
+< 	va\_list ap;
+<
+< 	va\_start(ap, format);
+< 	msg = g\_strdup\_vprintf(format, ap);
+< 	va\_end(ap);
+< 	dialog = gtk\_dialog\_new();
+< 	button = gtk\_button\_new\_with\_label(msg);
+< 	g\_free(msg);
+< 	gtk\_signal\_connect (GTK\_OBJECT (button), "clicked", GTK\_SIGNAL\_FUNC (destroy),dialog);
+< 	gtk\_container\_add (GTK\_CONTAINER (GTK\_DIALOG(dialog)->vbox), button);
+< 	gtk\_widget\_show (button);
+<
+< 	gtk\_widget\_show\_all(dialog);
+< //	gtk\_dialog\_run(GTK\_DIALOG(dialog));
+< //	gtk\_widget\_destroy(dialog);
+< }
+<
+
+
+MainWindow.h
+87d86
+< 	bool IsMessageShow;
+
+
+ProgramData.h
+52c52
+< 	uint8\_t flags;		//7 消息到来弹出提示: 6 图标,5 传输:4 enter:3 历史:2 日志:1 黑名单:0 共享
+---
+> 	uint8\_t flags;		//6 图标,5 传输:4 enter:3 历史:2 日志:1 黑名单:0 共享
+
+
+StatusIcon.h
+24d23
+< 	static void StatusIconActivate();
+33a33
+> 	static void StatusIconActivate();
+
+
+output.h
+38d37
+< void pop\_info1(GtkWidget**parent, const gchar 