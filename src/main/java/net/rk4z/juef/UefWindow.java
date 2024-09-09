package net.rk4z.juef;

import net.rk4z.juef.listener.UefViewListener;
import net.rk4z.juef.listener.UefWindowListener;

import static net.rk4z.juef.UefApp.logger;

public class UefWindow implements AutoCloseable {
    private long nativeWindowPtr;

    public UefWindow(String title, String url, int x, int y, int width, int height, int flags) {
        nativeWindowPtr = createWindow(title, url, x, y, width, height, flags);
    }

    public void setWindowListener(UefWindowListener listener) {
        setWindowListener(getNativeWindowPtr(), listener);
    }

    public void setViewListener(UefViewListener listener) {
        setViewListener(getNativeWindowPtr(), listener);
    }

    public void moveTo(int x, int y) {
        moveTo(getNativeWindowPtr(), x, y);
    }

    public void show() {
        show(getNativeWindowPtr());
    }

    public void hide() {
        hide(getNativeWindowPtr());
    }

//>------------------- Native methods --------------------<\\

    private native long createWindow(String title, String url, int x, int y, int width, int height, int flags);

    private native void setWindowListener(long windowPtr, UefWindowListener listener);

    private native void setViewListener(long windowPtr, UefViewListener listener);

    private native void moveTo(long windowPtr, int x, int y);

    private native void show(long windowPtr);

    private native void hide(long windowPtr);

    private native void destroy(long windowPtr);

//>------------------- Native methods -------------------<\\

    /**
     * @deprecated This is a low-level API that exposes internal native pointers.
     * It should not be used unless absolutely necessary.
     */
    public long getNativeWindowPtr() {
        return nativeWindowPtr;
    }

    @Override
    public void close() {
        if (nativeWindowPtr != 0) {
            logger.info("Closing window");
            destroy(nativeWindowPtr);
            nativeWindowPtr = 0;
        }
    }
}
