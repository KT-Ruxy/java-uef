#include "UefWindow.h"
#include <jni.h>
#include <AppCore/Window.h>
#include <AppCore/Overlay.h>
#include "UefApp.h"

extern "C" {
    using namespace ultralight;

    UefWindow::UefWindow(const char *title, const char *url, int x, int y, int width, int height, bool fullScreen, int flags)
        : javaWindowListener_(nullptr), javaViewListener_(nullptr) {
        window_ = Window::Create(app->main_monitor(), width, height, fullScreen, flags);
        window_->MoveTo(x, y);
        window_->SetTitle(title);
        window_->Show();
        window_->set_listener(this);

        overlay_ = Overlay::Create(window_, window_->width(), window_->height(), 0, 0);
        overlay_->view()->LoadURL(url);
        overlay_->view()->set_view_listener(this);
    }

    UefWindow::~UefWindow() {
        if (javaWindowListener_ != nullptr) {
            JNIEnv *env = javaWindowListener_->env;
            env->DeleteGlobalRef(javaWindowListener_->listener);
            delete javaWindowListener_;
        }

        if (javaViewListener_ != nullptr) {
            JNIEnv *env = javaViewListener_->env;
            env->DeleteGlobalRef(javaViewListener_->listener);
            delete javaViewListener_;
        }
    }

    void UefWindow::moveTo(int x, int y) const {
        window_->MoveTo(x, y);
    }

    void UefWindow::show() const {
        window_->Show();
    }

    void UefWindow::hide() const {
        window_->Hide();
    }

    void UefWindow::setJavaWindowListener(JavaListener *listener) {
        if (javaWindowListener_ != nullptr) {
            JNIEnv *env = javaWindowListener_->env;
            env->DeleteGlobalRef(javaWindowListener_->listener);
            delete javaWindowListener_;
        }
        javaWindowListener_ = listener;
    }

    void UefWindow::setJavaViewListener(JavaListener *listener) {
        if (javaViewListener_ != nullptr) {
            JNIEnv *env = javaViewListener_->env;
            env->DeleteGlobalRef(javaViewListener_->listener);
            delete javaViewListener_;
        }
        javaViewListener_ = listener;
    }

    void UefWindow::OnClose(Window *window) {
        if (javaWindowListener_ && javaWindowListener_->listener) {
            JNIEnv *env = javaWindowListener_->env;
            jclass listenerClass = env->GetObjectClass(javaWindowListener_->listener);
            jmethodID onCloseMethod = env->GetMethodID(listenerClass, "onClose", "()V");
            if (onCloseMethod) {
                env->CallVoidMethod(javaWindowListener_->listener, onCloseMethod);
            }
        }
    }

    void UefWindow::OnResize(Window *window, uint32_t width, uint32_t height) {
        overlay_->Resize(width, height);
        if (javaWindowListener_ && javaWindowListener_->listener) {
            JNIEnv *env = javaWindowListener_->env;
            jclass listenerClass = env->GetObjectClass(javaWindowListener_->listener);
            jmethodID onResizeMethod = env->GetMethodID(listenerClass, "onResize", "(II)V");
            if (onResizeMethod) {
                env->CallVoidMethod(javaWindowListener_->listener, onResizeMethod, width, height);
            }
        }
    }

    bool UefWindow::OnKeyEvent(const KeyEvent &evt) {
        if (javaWindowListener_ && javaWindowListener_->listener) {
            JNIEnv *env = javaWindowListener_->env;

            jclass listenerClass = env->GetObjectClass(javaWindowListener_->listener);

            jmethodID onKeyEventMethod = env->GetMethodID(listenerClass, "onKeyEvent", "(Lnet/rk4z/juef/event/UefKeyEvent;)Z");

            if (onKeyEventMethod) {
                jclass keyEventClass = env->FindClass("net/rk4z/juef/event/UefKeyEvent");
                jmethodID keyEventConstructor = env->GetMethodID(keyEventClass, "<init>", "(Lnet/rk4z/juef/event/UefKeyEvent$Type;IIZ)V");

                jclass keyEventTypeClass = env->FindClass("net/rk4z/juef/event/UefKeyEvent$Type");
                jfieldID typeFieldID;
                switch (evt.type) {
                    case KeyEvent::kType_KeyDown:
                        typeFieldID = env->GetStaticFieldID(keyEventTypeClass, "KeyDown", "Lnet/rk4z/juef/event/UefKeyEvent$Type;");
                        break;
                    case KeyEvent::kType_KeyUp:
                        typeFieldID = env->GetStaticFieldID(keyEventTypeClass, "KeyUp", "Lnet/rk4z/juef/event/UefKeyEvent$Type;");
                        break;
                    case KeyEvent::kType_RawKeyDown:
                        typeFieldID = env->GetStaticFieldID(keyEventTypeClass, "RawKeyDown", "Lnet/rk4z/juef/event/UefKeyEvent$Type;");
                        break;
                    case KeyEvent::kType_Char:
                        typeFieldID = env->GetStaticFieldID(keyEventTypeClass, "Char", "Lnet/rk4z/juef/event/UefKeyEvent$Type;");
                        break;
                    default:
                        return false;
                }

                jobject keyEventType = env->GetStaticObjectField(keyEventTypeClass, typeFieldID);

                jobject keyEvent = env->NewObject(keyEventClass, keyEventConstructor, keyEventType, evt.virtual_key_code, evt.native_key_code, evt.is_system_key);

                jboolean result = env->CallBooleanMethod(javaWindowListener_->listener, onKeyEventMethod, keyEvent);

                env->DeleteLocalRef(keyEvent);
                env->DeleteLocalRef(keyEventType);

                return result == JNI_TRUE;
            }
        }
        return false;
    }

    bool UefWindow::OnMouseEvent(const MouseEvent &evt) {
        if (javaWindowListener_ && javaWindowListener_->listener) {
            JNIEnv *env = javaWindowListener_->env;

            jclass listenerClass = env->GetObjectClass(javaWindowListener_->listener);

            jmethodID onMouseEventMethod = env->GetMethodID(listenerClass, "onMouseEvent", "(Lnet/rk4z/juef/event/UefMouseEvent;)Z");
            if (onMouseEventMethod) {
                jclass mouseEventClass = env->FindClass("net/rk4z/juef/event/UefMouseEvent");
                jmethodID mouseEventConstructor = env->GetMethodID(mouseEventClass, "<init>",
                                                                   "(Lnet/rk4z/juef/event/UefMouseEvent$Type;IILnet/rk4z/juef/event/UefMouseEvent$Button;)V");

                jclass mouseEventTypeClass = env->FindClass("net/rk4z/juef/event/UefMouseEvent$Type");
                jfieldID typeFieldID;
                switch (evt.type) {
                    case MouseEvent::kType_MouseMoved:
                        typeFieldID = env->GetStaticFieldID(mouseEventTypeClass, "MouseMoved", "Lnet/rk4z/juef/event/UefMouseEvent$Type;");
                        break;
                    case MouseEvent::kType_MouseDown:
                        typeFieldID = env->GetStaticFieldID(mouseEventTypeClass, "MouseDown", "Lnet/rk4z/juef/event/UefMouseEvent$Type;");
                        break;
                    case MouseEvent::kType_MouseUp:
                        typeFieldID = env->GetStaticFieldID(mouseEventTypeClass, "MouseUp", "Lnet/rk4z/juef/event/UefMouseEvent$Type;");
                        break;
                    default:
                        return false;
                }
                jobject mouseEventType = env->GetStaticObjectField(mouseEventTypeClass, typeFieldID);

                jclass mouseButtonClass = env->FindClass("net/rk4z/juef/event/UefMouseEvent$Button");
                jfieldID buttonFieldID;
                switch (evt.button) {
                    case MouseEvent::kButton_Left:
                        buttonFieldID = env->GetStaticFieldID(mouseButtonClass, "Left", "Lnet/rk4z/juef/event/UefMouseEvent$Button;");
                        break;
                    case MouseEvent::kButton_Middle:
                        buttonFieldID = env->GetStaticFieldID(mouseButtonClass, "Middle", "Lnet/rk4z/juef/event/UefMouseEvent$Button;");
                        break;
                    case MouseEvent::kButton_Right:
                        buttonFieldID = env->GetStaticFieldID(mouseButtonClass, "Right", "Lnet/rk4z/juef/event/UefMouseEvent$Button;");
                        break;
                    default:
                        buttonFieldID = env->GetStaticFieldID(mouseButtonClass, "None", "Lnet/rk4z/juef/event/UefMouseEvent$Button;");
                        break;
                }
                jobject mouseButton = env->GetStaticObjectField(mouseButtonClass, buttonFieldID);

                jobject mouseEvent = env->NewObject(mouseEventClass, mouseEventConstructor, mouseEventType, evt.x, evt.y, mouseButton);

                jboolean result = env->CallBooleanMethod(javaWindowListener_->listener, onMouseEventMethod, mouseEvent);

                env->DeleteLocalRef(mouseEvent);
                env->DeleteLocalRef(mouseEventType);
                env->DeleteLocalRef(mouseButton);

                return result == JNI_TRUE;
            }
        }
        return false;
    }

    bool UefWindow::OnScrollEvent(const ScrollEvent &evt) {
        if (javaWindowListener_ && javaWindowListener_->listener) {
            JNIEnv *env = javaWindowListener_->env;

            jclass listenerClass = env->GetObjectClass(javaWindowListener_->listener);

            jmethodID onScrollEventMethod = env->GetMethodID(listenerClass, "onScrollEvent", "(Lnet/rk4z/juef/event/UefScrollEvent;)Z");
            if (onScrollEventMethod) {
                jclass scrollEventClass = env->FindClass("net/rk4z/juef/event/UefScrollEvent");
                jmethodID scrollEventConstructor = env->GetMethodID(scrollEventClass, "<init>", "(Lnet/rk4z/juef/event/UefScrollEvent$Type;II)V");

                jclass scrollEventTypeClass = env->FindClass("net/rk4z/juef/event/UefScrollEvent$Type");
                jfieldID typeFieldID;
                switch (evt.type) {
                    case ScrollEvent::kType_ScrollByPixel:
                        typeFieldID = env->GetStaticFieldID(scrollEventTypeClass, "ScrollByPixel", "Lnet/rk4z/juef/event/UefScrollEvent$Type;");
                        break;
                    case ScrollEvent::kType_ScrollByPage:
                        typeFieldID = env->GetStaticFieldID(scrollEventTypeClass, "ScrollByPage", "Lnet/rk4z/juef/event/UefScrollEvent$Type;");
                        break;
                    default:
                        return false;
                }
                jobject scrollEventType = env->GetStaticObjectField(scrollEventTypeClass, typeFieldID);

                jobject scrollEvent = env->NewObject(scrollEventClass, scrollEventConstructor, scrollEventType, evt.delta_x, evt.delta_y);

                jboolean result = env->CallBooleanMethod(javaWindowListener_->listener, onScrollEventMethod, scrollEvent);

                env->DeleteLocalRef(scrollEvent);
                env->DeleteLocalRef(scrollEventType);

                return result == JNI_TRUE;
            }
        }
        return false;
    }

    void UefWindow::OnChangeCursor(View *caller, Cursor cursor) {
        window_->SetCursor(cursor);
        if (javaViewListener_ && javaViewListener_->listener) {
            JNIEnv *env = javaViewListener_->env;

            jclass listenerClass = env->GetObjectClass(javaViewListener_->listener);
            jmethodID onChangeCursorMethod = env->GetMethodID(listenerClass, "onChangeCursor", "(JLnet/rk4z/juef/event/UefCursor;)V");

            if (onChangeCursorMethod) {
                jclass uefCursorClass = env->FindClass("net/rk4z/juef/event/UefCursor");
                jmethodID fromValueMethod = env->GetStaticMethodID(uefCursorClass, "fromValue", "(I)Lnet/rk4z/juef/event/UefCursor;");
                jobject uefCursor = env->CallStaticObjectMethod(uefCursorClass, fromValueMethod, static_cast<int>(cursor));

                env->CallVoidMethod(javaViewListener_->listener, onChangeCursorMethod, reinterpret_cast<jlong>(caller), uefCursor);

                env->DeleteLocalRef(uefCursor);
            }
        }
    }

    void UefWindow::OnChangeTitle(View *caller, const String &title) {
        if (javaViewListener_ && javaViewListener_->listener) {
            JNIEnv *env = javaViewListener_->env;

            jclass listenerClass = env->GetObjectClass(javaViewListener_->listener);

            jmethodID onChangeTitleMethod = env->GetMethodID(listenerClass, "onChangeTitle", "(JLjava/lang/String;)V");

            if (onChangeTitleMethod) {
                jstring jTitle = env->NewStringUTF(title.utf8().data());

                env->CallVoidMethod(javaViewListener_->listener, onChangeTitleMethod, reinterpret_cast<jlong>(caller), jTitle);

                env->DeleteLocalRef(jTitle);
            }
        }
    }

    void UefWindow::OnChangeTooltip(View *caller, const String &tooltip) {
        if (javaViewListener_ && javaViewListener_->listener) {
            JNIEnv *env = javaViewListener_->env;

            jclass listenerClass = env->GetObjectClass(javaViewListener_->listener);

            jmethodID onChangeTooltipMethod = env->GetMethodID(listenerClass, "onChangeTooltip", "(JLjava/lang/String;)V");

            if (onChangeTooltipMethod) {
                jstring jTooltip = env->NewStringUTF(tooltip.utf8().data());

                env->CallVoidMethod(javaViewListener_->listener, onChangeTooltipMethod, reinterpret_cast<jlong>(caller), jTooltip);

                env->DeleteLocalRef(jTooltip);
            }
        }
    }

    void UefWindow::OnRequestClose(View *caller) {
        if (javaViewListener_ && javaViewListener_->listener) {
            JNIEnv *env = javaViewListener_->env;

            jclass listenerClass = env->GetObjectClass(javaViewListener_->listener);
            jmethodID onRequestCloseMethod = env->GetMethodID(listenerClass, "onRequestClose", "(J)V");

            if (onRequestCloseMethod) {
                env->CallVoidMethod(javaViewListener_->listener, onRequestCloseMethod, reinterpret_cast<jlong>(caller));
            }
        }
    }

    void UefWindow::OnAddConsoleMessage(View *caller, MessageSource source, MessageLevel level, const String &message, uint32_t line_number, uint32_t column_number,
                                        const String &source_id) {
        if (javaViewListener_ && javaViewListener_->listener) {
            JNIEnv *env = javaViewListener_->env;

            jclass listenerClass = env->GetObjectClass(javaViewListener_->listener);
            jmethodID onAddConsoleMessageMethod = env->GetMethodID(listenerClass, "onAddConsoleMessage",
                                                                   "(JLnet/rk4z/juef/event/UefMessageSource;Lnet/rk4z/juef/event/UefMessageLevel;Ljava/lang/String;IILjava/lang/String;)V");

            if (onAddConsoleMessageMethod) {
                jclass sourceClass = env->FindClass("net/rk4z/juef/event/UefMessageSource");
                jmethodID fromValueSourceMethod = env->GetStaticMethodID(sourceClass, "fromValue", "(I)Lnet/rk4z/juef/event/UefMessageSource;");
                jobject uefSource = env->CallStaticObjectMethod(sourceClass, fromValueSourceMethod, static_cast<int>(source));

                jclass levelClass = env->FindClass("net/rk4z/juef/event/UefMessageLevel");
                jmethodID fromValueLevelMethod = env->GetStaticMethodID(levelClass, "fromValue", "(I)Lnet/rk4z/juef/event/UefMessageLevel;");
                jobject uefLevel = env->CallStaticObjectMethod(levelClass, fromValueLevelMethod, static_cast<int>(level));

                jstring jMessage = env->NewStringUTF(message.utf8().data());
                jstring jSourceId = env->NewStringUTF(source_id.utf8().data());

                env->CallVoidMethod(javaViewListener_->listener, onAddConsoleMessageMethod, reinterpret_cast<jlong>(caller), uefSource, uefLevel, jMessage,
                                    line_number, column_number, jSourceId);

                env->DeleteLocalRef(uefSource);
                env->DeleteLocalRef(uefLevel);
                env->DeleteLocalRef(jMessage);
                env->DeleteLocalRef(jSourceId);
            }
        }
    }

    void UefWindow::OnChangeURL(View *caller, const String &url) {
        if (javaViewListener_ && javaViewListener_->listener) {
            JNIEnv *env = javaViewListener_->env;

            jclass listenerClass = env->GetObjectClass(javaViewListener_->listener);
            jmethodID onChangeURLMethod = env->GetMethodID(listenerClass, "onChangeURL", "(JLjava/lang/String;)V");

            if (onChangeURLMethod) {
                jstring jURL = env->NewStringUTF(url.utf8().data());
                env->CallVoidMethod(javaViewListener_->listener, onChangeURLMethod, reinterpret_cast<jlong>(caller), jURL);
                env->DeleteLocalRef(jURL);
            }
        }
    }

    RefPtr<View> UefWindow::OnCreateChildView(View *caller, const String &opener_url, const String &target_url, bool is_popup, const IntRect &popup_rect) {
        if (javaViewListener_ && javaViewListener_->listener) {
            JNIEnv *env = javaViewListener_->env;

            jclass listenerClass = env->GetObjectClass(javaViewListener_->listener);
            jmethodID onCreateChildViewMethod = env->GetMethodID(listenerClass, "onCreateChildView",
                                                                 "(JLjava/lang/String;Ljava/lang/String;ZLnet/rk4z/juef/util/IntRect;)J");

            if (onCreateChildViewMethod) {
                jstring jOpenerURL = env->NewStringUTF(opener_url.utf8().data());
                jstring jTargetURL = env->NewStringUTF(target_url.utf8().data());

                jclass intRectClass = env->FindClass("net/rk4z/juef/util/IntRect");
                jmethodID intRectConstructor = env->GetMethodID(intRectClass, "<init>", "(IIII)V");
                jobject jPopupRect = env->NewObject(intRectClass, intRectConstructor, popup_rect.left, popup_rect.top, popup_rect.right, popup_rect.bottom);

                jlong newViewPtr = env->CallLongMethod(javaViewListener_->listener, onCreateChildViewMethod, reinterpret_cast<jlong>(caller), jOpenerURL,
                                                       jTargetURL, static_cast<jboolean>(is_popup), jPopupRect);

                env->DeleteLocalRef(jOpenerURL);
                env->DeleteLocalRef(jTargetURL);
                env->DeleteLocalRef(jPopupRect);

                if (newViewPtr != 0) {
                    return reinterpret_cast<View *>(newViewPtr);
                }
            }
        }

        ViewConfig config;
        RefPtr<Renderer> renderer = app->renderer();
        RefPtr<Session> session = renderer->default_session();

        return renderer->CreateView(caller->width(), caller->height(), config, session);
    }

    RefPtr<View> UefWindow::OnCreateInspectorView(View *caller, bool is_local, const String &inspected_url) {
        if (javaViewListener_ && javaViewListener_->listener) {
            JNIEnv *env = javaViewListener_->env;

            jclass listenerClass = env->GetObjectClass(javaViewListener_->listener);
            jmethodID onCreateInspectorViewMethod = env->GetMethodID(listenerClass, "onCreateInspectorView", "(JZLjava/lang/String;)J");

            if (onCreateInspectorViewMethod) {
                jstring jInspectedURL = env->NewStringUTF(inspected_url.utf8().data());

                jlong newViewPtr = env->CallLongMethod(javaViewListener_->listener, onCreateInspectorViewMethod, reinterpret_cast<jlong>(caller),
                                                       static_cast<jboolean>(is_local), jInspectedURL);

                env->DeleteLocalRef(jInspectedURL);

                if (newViewPtr != 0) {
                    return reinterpret_cast<View *>(newViewPtr);
                }
            }
        }

        ViewConfig config;
        RefPtr<Renderer> renderer = app->renderer();
        RefPtr<Session> session = renderer->default_session();

        return renderer->CreateView(caller->width(), caller->height(), config, session);
    }

    JNIEXPORT jlong JNICALL Java_net_rk4z_juef_UefWindow_createWindow(JNIEnv *env, jobject obj, jstring title, jstring url, jint x, jint y, jint width, jint
                                                                      height, jboolean fullScreen, jint flags) {
        const char *c_title = env->GetStringUTFChars(title, nullptr);
        const char *c_url = env->GetStringUTFChars(url, nullptr);

        auto *window = new UefWindow(c_title, c_url, x, y, width, height, fullScreen, flags);

        env->ReleaseStringUTFChars(title, c_title);
        env->ReleaseStringUTFChars(url, c_url);

        return reinterpret_cast<jlong>(window);
    }

    JNIEXPORT void JNICALL Java_net_rk4z_juef_UefWindow_setWindowListener(JNIEnv *env, jobject obj, jlong window_ptr, jobject listener) {
        auto *window = reinterpret_cast<UefWindow *>(window_ptr);

        auto *javaListener = new JavaListener();
        javaListener->listener = env->NewGlobalRef(listener);
        javaListener->env = env;

        window->setJavaWindowListener(javaListener);
    }

    JNIEXPORT void JNICALL Java_net_rk4z_juef_UefWindow_setViewListener(JNIEnv *env, jobject obj, jlong window_ptr, jobject listener) {
        auto *window = reinterpret_cast<UefWindow *>(window_ptr);

        auto *javaListener = new JavaListener();
        javaListener->listener = env->NewGlobalRef(listener);
        javaListener->env = env;

        window->setJavaViewListener(javaListener);
    }

    JNIEXPORT void JNICALL Java_net_rk4z_juef_UefWindow_show(JNIEnv *env, jobject obj, jlong window_ptr) {
        auto *window = reinterpret_cast<UefWindow *>(window_ptr);
        window->show();
    }

    JNIEXPORT void JNICALL Java_net_rk4z_juef_UefWindow_hide(JNIEnv *env, jobject obj, jlong window_ptr) {
        auto *window = reinterpret_cast<UefWindow *>(window_ptr);
        window->hide();
    }

    JNIEXPORT void JNICALL Java_net_rk4z_juef_UefWindow_destroy(JNIEnv *env, jobject obj, jlong window_ptr) {
        auto *window = reinterpret_cast<UefWindow *>(window_ptr);
        delete window;
    }

    JNIEXPORT void JNICALL Java_net_rk4z_juef_UefWindow_moveTo(JNIEnv *env, jobject obj, jlong window_ptr, jint x, jint y) {
        auto *window = reinterpret_cast<UefWindow *>(window_ptr);
        window->moveTo(x, y);
    }
}
