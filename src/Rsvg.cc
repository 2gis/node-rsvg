#include "Rsvg.h"
#include "RsvgCairo.h"
#include "Enums.h"
#include <node_buffer.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <string>
#include <iostream>
#include <cmath>

#ifdef _MSC_VER
#include "win32-math.h"
#endif

using namespace v8;
using namespace node;

cairo_status_t GetDataChunks(void* closure, const unsigned char* chunk, unsigned int length) {
    std::string* data = reinterpret_cast<std::string*>(closure);
    data->append(reinterpret_cast<const char *>(chunk), length);
    return CAIRO_STATUS_SUCCESS;
}

Nan::Persistent<Function> Rsvg::constructor;

#if NODE_VERSION_AT_LEAST(6, 0, 0)
void Rsvg::propGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().Set(info.Data());
}
#endif

Rsvg::Rsvg(RsvgHandle* const handle) : _handle(handle) {}

Rsvg::~Rsvg() {
    g_object_unref(G_OBJECT(_handle));
}

NAN_MODULE_INIT(Rsvg::Init) {

#if !GLIB_CHECK_VERSION(2, 36, 0)
    // Initialize GObject types.
    g_type_init();
#endif

    // Prepare constructor template.
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New<String>("Rsvg").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Add methods to prototype.
    Nan::SetPrototypeMethod(tpl, "getBaseURI", GetBaseURI);
    Nan::SetPrototypeMethod(tpl, "getBaseURI", GetBaseURI);
    Nan::SetPrototypeMethod(tpl, "setBaseURI", SetBaseURI);
    Nan::SetPrototypeMethod(tpl, "getDPI", GetDPI);
    Nan::SetPrototypeMethod(tpl, "setDPI", SetDPI);
    Nan::SetPrototypeMethod(tpl, "getDPIX", GetDPIX);
    Nan::SetPrototypeMethod(tpl, "setDPIX", SetDPIX);
    Nan::SetPrototypeMethod(tpl, "getDPIY", GetDPIY);
    Nan::SetPrototypeMethod(tpl, "setDPIY", SetDPIY);
    Nan::SetPrototypeMethod(tpl, "getWidth", GetWidth);
    Nan::SetPrototypeMethod(tpl, "getHeight", GetHeight);
    Nan::SetPrototypeMethod(tpl, "write", Write);
    Nan::SetPrototypeMethod(tpl, "close", Close);
    Nan::SetPrototypeMethod(tpl, "dimensions", Dimensions);
    Nan::SetPrototypeMethod(tpl, "hasElement", HasElement);
    Nan::SetPrototypeMethod(tpl, "autocrop", Autocrop);
    Nan::SetPrototypeMethod(tpl, "render", Render);
    // Export class.
    Local<Function> tplFunc = Nan::GetFunction(tpl).ToLocalChecked();
    constructor.Reset(tplFunc);
    Nan::Set(target, Nan::New("Rsvg").ToLocalChecked(), tplFunc);
}

NAN_METHOD(Rsvg::New) {
    Nan::HandleScope scope;

    if (ARGVAR.IsConstructCall()) {
        // Invoked as constructor: `new Rsvg(...)`
        RsvgHandle* handle;
        if (Buffer::HasInstance(ARGVAR[0])) {
            const guint8* buffer =
                reinterpret_cast<guint8*>(Buffer::Data(ARGVAR[0]));
            gsize length = Buffer::Length(ARGVAR[0]);

            GError* error = NULL;
            handle = rsvg_handle_new_from_data(buffer, length, &error);

            if (error) {
                Nan::ThrowError(error->message);
                g_error_free(error);
                return ARGVAR.GetReturnValue().Set(Nan::Undefined());
            }
        } else {
            handle = rsvg_handle_new_with_flags(RSVG_HANDLE_FLAG_KEEP_IMAGE_DATA);
        }
        // Error handling.
        if (!handle) {
            Nan::ThrowError("Unable to create RsvgHandle instance.");
            return ARGVAR.GetReturnValue().Set(Nan::Undefined());
        }
        // Create object.
        Rsvg* obj = new Rsvg(handle);
        obj->Wrap(ARGVAR.This());
        ARGVAR.GetReturnValue().Set(ARGVAR.This());
    } else {
        // Invoked as plain function `Rsvg(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = { ARGVAR[0] };
        ARGVAR.GetReturnValue().Set(CREATE_FUNC(constructor));
    }
}

NAN_METHOD(Rsvg::GetBaseURI) {
    Nan::HandleScope scope;
    ARGVAR.GetReturnValue().Set(GetStringProperty("base-uri", ARGVAR));
}

NAN_METHOD(Rsvg::SetBaseURI) {
    Nan::HandleScope scope;
    SetStringProperty("base-uri", ARGVAR);
    ARGVAR.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::GetDPI) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());
    gdouble dpiX = 0;
    gdouble dpiY = 0;
    g_object_get(
            G_OBJECT(obj->_handle),
            "dpi-x", &dpiX,
            "dpi-y", &dpiY,
            NULL
            );

    Handle<ObjectTemplate> dpi = Nan::New<ObjectTemplate>();
    dpi->Set(Nan::New("x").ToLocalChecked(), Nan::New<Number>(dpiX));
    dpi->Set(Nan::New("y").ToLocalChecked(), Nan::New<Number>(dpiY));

    ARGVAR.GetReturnValue().Set(dpi->NewInstance());
}

NAN_METHOD(Rsvg::SetDPI) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());

    gdouble x = ARGVAR[0]->NumberValue();
    if (std::isnan(x)) {
        x = 0;
    }

    gdouble y = x;
    if (ARGVAR[1]->IsNumber()) {
        y = ARGVAR[1]->NumberValue();
        if (std::isnan(y)) {
            y = 0;
        }
    }

    rsvg_handle_set_dpi_x_y(obj->_handle, x, y);
    ARGVAR.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::GetDPIX) {
    Nan::HandleScope scope;
    ARGVAR.GetReturnValue().Set(GetNumberProperty("dpi-x", ARGVAR));
}

NAN_METHOD(Rsvg::SetDPIX) {
    Nan::HandleScope scope;
    SetNumberProperty("dpi-x", ARGVAR);
    ARGVAR.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::GetDPIY) {
    Nan::HandleScope scope;
    ARGVAR.GetReturnValue().Set(GetNumberProperty("dpi-y", ARGVAR));
}

NAN_METHOD(Rsvg::SetDPIY) {
    Nan::HandleScope scope;
    SetNumberProperty("dpi-y", ARGVAR);
    ARGVAR.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::GetWidth) {
    Nan::HandleScope scope;
    ARGVAR.GetReturnValue().Set(GetIntegerProperty("width", ARGVAR));
}

NAN_METHOD(Rsvg::GetHeight) {
    Nan::HandleScope scope;
    ARGVAR.GetReturnValue().Set(GetIntegerProperty("height", ARGVAR));
}

NAN_METHOD(Rsvg::Write) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());
    if (Buffer::HasInstance(ARGVAR[0])) {
        const guchar* buffer =
            reinterpret_cast<guchar*>(Buffer::Data(ARGVAR[0]));
        gsize length = Buffer::Length(ARGVAR[0]);

        GError* error = NULL;
        gboolean success = rsvg_handle_write(obj->_handle, buffer, length, &error);

        if (error) {
            Nan::ThrowError(error->message);
            g_error_free(error);
        } else if (!success) {
            Nan::ThrowError("Failed to write data.");
        }
    } else {
        Nan::ThrowError("Invalid argument: buffer");
    }
    ARGVAR.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::Close) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());

    GError* error = NULL;
    gboolean success = rsvg_handle_close(obj->_handle, &error);

    if (error) {
        Nan::ThrowError(error->message);
        g_error_free(error);
    } else if (!success) {
        Nan::ThrowError("Failed to close.");
    }
    ARGVAR.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::Dimensions) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());

    const char* id = NULL;
    String::Utf8Value idArg(ARGVAR[0]);
    if (!(ARGVAR[0]->IsUndefined() || ARGVAR[0]->IsNull())) {
        id = *idArg;
        if (!id) {
            Nan::ThrowError("Invalid argument: id");
            return ARGVAR.GetReturnValue().Set(Nan::Undefined());
        }
    }

    RsvgPositionData _position = { 0, 0 };
    RsvgDimensionData _dimensions = { 0, 0, 0, 0 };

    gboolean hasPosition = rsvg_handle_get_position_sub(obj->_handle, &_position, id);
    gboolean hasDimensions = rsvg_handle_get_dimensions_sub(obj->_handle, &_dimensions, id);

    if (hasPosition || hasDimensions) {
        Handle<ObjectTemplate> dimensions = Nan::New<ObjectTemplate>();
        if (hasPosition) {
            dimensions->Set(Nan::New("x").ToLocalChecked(), Nan::New<Integer>(_position.x));
            dimensions->Set(Nan::New("y").ToLocalChecked(), Nan::New<Integer>(_position.y));
        }
        if (hasDimensions) {
            dimensions->Set(Nan::New("width").ToLocalChecked(), Nan::New<Integer>(_dimensions.width));
            dimensions->Set(Nan::New("height").ToLocalChecked(), Nan::New<Integer>(_dimensions.height));
        }
        ARGVAR.GetReturnValue().Set(dimensions->NewInstance());
    } else {
        ARGVAR.GetReturnValue().Set(Nan::Null());
    }
}

NAN_METHOD(Rsvg::HasElement) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());

    const char* id = NULL;
    String::Utf8Value idArg(ARGVAR[0]);
    if (!(ARGVAR[0]->IsUndefined() || ARGVAR[0]->IsNull())) {
        id = *idArg;
        if (!id) {
            Nan::ThrowError("Invalid argument: id");
            return ARGVAR.GetReturnValue().Set(Nan::Undefined());
        }
    }

    gboolean exists = rsvg_handle_has_sub(obj->_handle, id);
    ARGVAR.GetReturnValue().Set(Nan::New<Boolean>(exists));
}

NAN_METHOD(Rsvg::Render) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());

    int width = ARGVAR[0]->Int32Value();
    int height = ARGVAR[1]->Int32Value();

    if (width <= 0) {
        Nan::ThrowError("Expected width > 0.");
        return ARGVAR.GetReturnValue().Set(Nan::Undefined());
    }
    if (height <= 0) {
        Nan::ThrowError("Expected height > 0.");
        return ARGVAR.GetReturnValue().Set(Nan::Undefined());
    }

    String::Utf8Value formatArg(ARGVAR[2]);
    const char* formatString = *formatArg;
    render_format_t renderFormat = RenderFormatFromString(formatString);
    cairo_format_t pixelFormat = CAIRO_FORMAT_INVALID;
    if (renderFormat == RENDER_FORMAT_RAW ||
            renderFormat == RENDER_FORMAT_PNG) {
        pixelFormat = CAIRO_FORMAT_ARGB32;
    } else if (renderFormat == RENDER_FORMAT_JPEG) {
        Nan::ThrowError("Format not supported: JPEG");
        return ARGVAR.GetReturnValue().Set(Nan::Undefined());
    } else if (
            renderFormat == RENDER_FORMAT_SVG ||
            renderFormat == RENDER_FORMAT_PDF) {
        pixelFormat = CAIRO_FORMAT_INVALID;
    } else if (renderFormat == RENDER_FORMAT_VIPS) {
        Nan::ThrowError("Format not supported: VIPS");
        return ARGVAR.GetReturnValue().Set(Nan::Undefined());
    } else {
        renderFormat = RENDER_FORMAT_RAW;
        pixelFormat = CairoFormatFromString(formatString);
        if (pixelFormat == CAIRO_FORMAT_INVALID) {
            Nan::ThrowError("Invalid argument: format");
            return ARGVAR.GetReturnValue().Set(Nan::Undefined());
        }
    }

    const char* id = NULL;
    String::Utf8Value idArg(ARGVAR[3]);
    if (!(ARGVAR[3]->IsUndefined() || ARGVAR[3]->IsNull())) {
        id = *idArg;
        if (!id) {
            Nan::ThrowError("Invalid argument: id");
            return ARGVAR.GetReturnValue().Set(Nan::Undefined());
        }
        if (!rsvg_handle_has_sub(obj->_handle, id)) {
            Nan::ThrowError("SVG element with given id does not exists.");
            return ARGVAR.GetReturnValue().Set(Nan::Undefined());
        }
    }

    RsvgPositionData position = { 0, 0 };
    RsvgDimensionData dimensions = { 0, 0, 0, 0 };

    if (!rsvg_handle_get_position_sub(obj->_handle, &position, id)) {
        Nan::ThrowError("Could not get position of SVG element with given id.");
        return ARGVAR.GetReturnValue().Set(Nan::Undefined());
    }

    if (!rsvg_handle_get_dimensions_sub(obj->_handle, &dimensions, id)) {
        Nan::ThrowError("Could not get dimensions of SVG element or whole image.");
        return ARGVAR.GetReturnValue().Set(Nan::Undefined());
    }
    if (dimensions.width <= 0 || dimensions.height <= 0) {
        Nan::ThrowError("Got invalid dimensions of SVG element or whole image.");
        return ARGVAR.GetReturnValue().Set(Nan::Undefined());
    }

    std::string data;
    cairo_surface_t* surface;

    if (renderFormat == RENDER_FORMAT_SVG) {
        surface = cairo_svg_surface_create_for_stream(GetDataChunks, &data, width, height);
        cairo_svg_surface_restrict_to_version(surface, CAIRO_SVG_VERSION_1_1);
    } else if (renderFormat == RENDER_FORMAT_PDF) {
        surface = cairo_pdf_surface_create_for_stream(GetDataChunks, &data, width, height);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0)
        cairo_pdf_surface_restrict_to_version(surface, CAIRO_PDF_VERSION_1_4);
#endif
    } else {
        surface = cairo_image_surface_create(pixelFormat, width, height);
    }

    cairo_t* cr = cairo_create(surface);
    // printf(
    // 	"%s: (%d, %d) %dx%d, render: %dx%d\n",
    // 	id ? id : "SVG",
    // 	position.x,
    // 	position.y,
    // 	dimensions.width,
    // 	dimensions.height,
    // 	width,
    // 	height
    // );
    double scaleX = double(width) / double(dimensions.width);
    double scaleY = double(height) / double(dimensions.height);
    // printf("scale=%.3fx%.3f\n", scaleX, scaleY);
    double scale = MIN(scaleX, scaleY);
    int bboxWidth = round(scale * dimensions.width);
    int bboxHeight = round(scale * dimensions.height);
    int bboxX = (width - bboxWidth) / 2;
    int bboxY = (height - bboxHeight) / 2;
    // printf("bbox=(%d, %d) %dx%d\n", bboxX, bboxY, bboxWidth, bboxHeight);
    cairo_translate(cr, bboxX, bboxY);
    cairo_scale(cr, scale, scale);
    cairo_translate(cr, -position.x, -position.y);

    gboolean success;
    if (id) {
        success = rsvg_handle_render_cairo_sub(obj->_handle, cr, id);
    } else {
        success = rsvg_handle_render_cairo(obj->_handle, cr);
    }
    cairo_surface_flush(surface);

    cairo_status_t status = cairo_status(cr);
    if (status || !success) {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);

        Nan::ThrowError(
                status ? cairo_status_to_string(status) : "Failed to render image."
                );
        return ARGVAR.GetReturnValue().Set(Nan::Undefined());
    }

    int stride = -1;
    if (renderFormat == RENDER_FORMAT_RAW) {
        stride = cairo_image_surface_get_stride(surface);
        data.append(
                reinterpret_cast<char*>(cairo_image_surface_get_data(surface)),
                stride * height
                );
    } else if (renderFormat == RENDER_FORMAT_PNG) {
        cairo_surface_write_to_png_stream(surface, GetDataChunks, &data);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    if (renderFormat == RENDER_FORMAT_RAW &&
            pixelFormat == CAIRO_FORMAT_ARGB32 &&
            stride != width * 4) {
        Nan::ThrowError(
                "Rendered with invalid stride (byte size of row) for ARGB32 format."
                );
        return ARGVAR.GetReturnValue().Set(Nan::Undefined());
    }

    Handle<ObjectTemplate> image = CREATE_OBJ();
    if (renderFormat == RENDER_FORMAT_SVG) {
        PROP_SET(image, "data", Nan::New<String>(data.c_str()).ToLocalChecked());
    } else {
        char* strbuf = new char[data.length()+1];
        data.copy(strbuf, data.length());

        Nan::MaybeLocal<v8::Object> buffer = Nan::NewBuffer(strbuf, data.length());
        PROP_SET(image, "data", buffer.ToLocalChecked());
    }
    
    PROP_SET(image, "format", RenderFormatToString(renderFormat));
    if (pixelFormat != CAIRO_FORMAT_INVALID) {
        PROP_SET(image, "pixelFormat", CairoFormatToString(pixelFormat));
    }

    PROP_SET(image, "width", Nan::New<Integer>(width));
    PROP_SET(image, "height", Nan::New<Integer>(height));
    if (stride != -1) {
        PROP_SET(image, "stride", Nan::New<Integer>(stride));
    }

    ARGVAR.GetReturnValue().Set(image->NewInstance());
}

v8::Local<v8::String> Rsvg::GetStringProperty (const char* property, const ARGTYPE& ARGVAR) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());
    gchar* value = NULL;
    g_object_get(G_OBJECT(obj->_handle), property, &value, NULL);
    if (value) {
        v8::Local<String> result = Nan::New<String>(value).ToLocalChecked();
        g_free(value);
        return result;
    }
     // TODO: possibly breaking change: returning empty string instead of null!
    return Nan::New<String>("").ToLocalChecked();
}

void Rsvg::SetStringProperty (const char* property, const ARGTYPE& ARGVAR) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());
    gchar* value = NULL;
    String::Utf8Value arg0(ARGVAR[0]);
    if (!(ARGVAR[0]->IsNull() || ARGVAR[0]->IsUndefined())) {
        value = *arg0;
    }
    g_object_set(G_OBJECT(obj->_handle), property, value, NULL);
}

v8::Local<v8::Number> Rsvg::GetNumberProperty (const char* property, const ARGTYPE& ARGVAR) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());
    gdouble value = 0;
    g_object_get(G_OBJECT(obj->_handle), property, &value, NULL);
    return Nan::New<Number>(value);
}

void Rsvg::SetNumberProperty (const char* property, const ARGTYPE& ARGVAR) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());
    gdouble value = ARGVAR[0]->NumberValue();
    if (std::isnan(value)) {
        value = 0;
    }
    g_object_set(G_OBJECT(obj->_handle), property, value, NULL);
}

v8::Local<v8::Integer> Rsvg::GetIntegerProperty (const char* property, const ARGTYPE& ARGVAR) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());
    gint value = 0;
    g_object_get(G_OBJECT(obj->_handle), property, &value, NULL);
    return Nan::New<Integer>(value);
}

void Rsvg::SetIntegerProperty (const char* property, const ARGTYPE& ARGVAR) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(ARGVAR.This());
    gint value = ARGVAR[0]->Int32Value();
    g_object_set(G_OBJECT(obj->_handle), property, value, NULL);
}

NODE_MODULE(rsvg, Rsvg::Init)
