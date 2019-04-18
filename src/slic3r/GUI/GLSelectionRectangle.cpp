#include "GLSelectionRectangle.hpp"
#include "Camera.hpp"
#include "3DScene.hpp"
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#include "GLCanvas3D.hpp"
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#include <GL/glew.h>

namespace Slic3r {
namespace GUI {

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void GLSelectionRectangle::start_dragging(const Vec2d& mouse_position, EState status)
{
    if (is_active() || (status == Off))
        return;

    m_status = status;
    m_start_corner = mouse_position;
    m_end_corner = mouse_position;
}

void GLSelectionRectangle::dragging(const Vec2d& mouse_position)
{
    if (!is_active())
        return;
    
    m_end_corner = mouse_position;
}

std::vector<unsigned int> GLSelectionRectangle::stop_dragging(const GLCanvas3D& canvas, const std::vector<Vec3d>& points)
{
    std::vector<unsigned int> out;

    if (!is_active())
        return out;

    m_status = Off;

    const Camera& camera = canvas.get_camera();
    const std::array<int, 4>& viewport = camera.get_viewport();
    const Transform3d& modelview_matrix = camera.get_view_matrix();
    const Transform3d& projection_matrix = camera.get_projection_matrix();

    // bounding box created from the rectangle corners - will take care of order of the corners
    BoundingBox rectangle(Points{ Point(m_start_corner.cast<int>()), Point(m_end_corner.cast<int>()) });

    // Iterate over all points and determine whether they're in the rectangle.
    for (unsigned int i = 0; i<points.size(); ++i) {
        const Vec3d& point = points[i];
        GLdouble out_x, out_y, out_z;
        ::gluProject((GLdouble)point(0), (GLdouble)point(1), (GLdouble)point(2), (GLdouble*)modelview_matrix.data(), (GLdouble*)projection_matrix.data(), (GLint*)viewport.data(), &out_x, &out_y, &out_z);
        out_y = canvas.get_canvas_size().get_height() - out_y;

        if (rectangle.contains(Point(out_x, out_y)))
            out.push_back(i);
    }

    return out;
}

void GLSelectionRectangle::stop_dragging()
{
    if (!is_active())
        return;

    m_status = Off;
}

void GLSelectionRectangle::render(const GLCanvas3D& canvas) const
{
    if (m_status == Off)
        return;

    float zoom = canvas.get_camera().zoom;
    float inv_zoom = (zoom != 0.0f) ? 1.0f / zoom : 0.0f;

    Size cnv_size = canvas.get_canvas_size();
    float cnv_half_width = 0.5f * (float)cnv_size.get_width();
    float cnv_half_height = 0.5f * (float)cnv_size.get_height();
    if ((cnv_half_width == 0.0f) || (cnv_half_height == 0.0f))
        return;

    Vec2d start(m_start_corner(0) - cnv_half_width, cnv_half_height - m_start_corner(1));
    Vec2d end(m_end_corner(0) - cnv_half_width, cnv_half_height - m_end_corner(1));

    float left = (float)std::min(start(0), end(0)) * inv_zoom;
    float top = (float)std::max(start(1), end(1)) * inv_zoom;
    float right = (float)std::max(start(0), end(0)) * inv_zoom;
    float bottom = (float)std::min(start(1), end(1)) * inv_zoom;

    glsafe(::glLineWidth(1.5f));
    float color[3];
    color[0] = (m_status == Select) ? 0.3f : 1.0f;
    color[1] = (m_status == Select) ? 1.0f : 0.3f;
    color[2] = 0.3f;
    glsafe(::glColor3fv(color));

    glsafe(::glDisable(GL_DEPTH_TEST));

    glsafe(::glPushMatrix());
    glsafe(::glLoadIdentity());

    glsafe(::glPushAttrib(GL_ENABLE_BIT));
    glsafe(::glLineStipple(4, 0xAAAA));
    glsafe(::glEnable(GL_LINE_STIPPLE));

    ::glBegin(GL_LINE_LOOP);
    ::glVertex2f((GLfloat)left, (GLfloat)bottom);
    ::glVertex2f((GLfloat)right, (GLfloat)bottom);
    ::glVertex2f((GLfloat)right, (GLfloat)top);
    ::glVertex2f((GLfloat)left, (GLfloat)top);
    glsafe(::glEnd());

    glsafe(::glPopAttrib());

    glsafe(::glPopMatrix());
}

//void GLSelectionRectangle::start_dragging(const Vec2d& mouse_position, float width, float height, EState status)
//{
//    if (is_active() || status == Off)
//        return;
//
//    m_width = width;
//    m_height = height;
//    m_status = status;
//    m_start_corner = mouse_position;
//    m_end_corner = mouse_position;
//}
//
//
//void GLSelectionRectangle::dragging(const Vec2d& mouse_position)
//{
//    if (is_active())
//        m_end_corner = mouse_position;
//}
//
//std::vector<unsigned int> GLSelectionRectangle::end_dragging(const Camera& camera, const std::vector<Vec3d>& points)
//{
//    if (!is_active())
//        return std::vector<unsigned int>();
//
//    m_status = Off;
//    std::vector<unsigned int> out;
//
//    const std::array<int, 4>& viewport = camera.get_viewport();
//    const Transform3d& modelview_matrix = camera.get_view_matrix();
//    const Transform3d& projection_matrix = camera.get_projection_matrix();
//
//    // bounding box created from the rectangle corners - will take care of order of the corners
//    BoundingBox rectangle(Points{ Point(m_start_corner.cast<int>()), Point(m_end_corner.cast<int>()) });
//
//    // Iterate over all points and determine whether they're in the rectangle.
//    for (unsigned int i = 0; i<points.size(); ++i) {
//        const Vec3d& point = points[i];
//        GLdouble out_x, out_y, out_z;
//        ::gluProject((GLdouble)point(0), (GLdouble)point(1), (GLdouble)point(2), (GLdouble*)modelview_matrix.data(), (GLdouble*)projection_matrix.data(), (GLint*)viewport.data(), &out_x, &out_y, &out_z);
//        out_y = m_height - out_y;
//
//        if (rectangle.contains(Point(out_x, out_y)))
//            out.push_back(i);
//    }
//
//    return out;
//}
//
//void GLSelectionRectangle::render() const
//{
//    float render_color[3];
//
//    switch (m_status) {
//    case Off: return;
//        case SlaSelect: render_color[0] = 0.f;
//                        render_color[1] = 1.f;
//                        render_color[2] = 0.f;
//                        break;
//        case SlaDeselect: render_color[0] = 1.f;
//                          render_color[1] = 0.3f;
//                          render_color[2] = 0.3f;
//                          break;
//    }
//
//    glsafe(::glColor3fv(render_color));
//    glsafe(::glLineWidth(1.5f));
//
//    glsafe(::glPushAttrib(GL_TRANSFORM_BIT));   // remember current MatrixMode
//
//    glsafe(::glMatrixMode(GL_MODELVIEW));       // cache modelview matrix and set to identity
//    glsafe(::glPushMatrix());
//    glsafe(::glLoadIdentity());
//
//    glsafe(::glMatrixMode(GL_PROJECTION));      // cache projection matrix and set to identity
//    glsafe(::glPushMatrix());
//    glsafe(::glLoadIdentity());
//
//    glsafe(::glOrtho(0.f, m_width, m_height, 0.f, -1.f, 1.f)); // set projection matrix so that world coords = window coords
//
//    // render the selection  rectangle (window coordinates):
//    glsafe(::glPushAttrib(GL_ENABLE_BIT));
//    glsafe(::glLineStipple(4, 0xAAAA));
//    glsafe(::glEnable(GL_LINE_STIPPLE));
//
//    ::glBegin(GL_LINE_LOOP);
//    ::glVertex3f((GLfloat)m_start_corner(0), (GLfloat)m_start_corner(1), (GLfloat)0.5f);
//    ::glVertex3f((GLfloat)m_end_corner(0), (GLfloat)m_start_corner(1), (GLfloat)0.5f);
//    ::glVertex3f((GLfloat)m_end_corner(0), (GLfloat)m_end_corner(1), (GLfloat)0.5f);
//    ::glVertex3f((GLfloat)m_start_corner(0), (GLfloat)m_end_corner(1), (GLfloat)0.5f);
//    glsafe(::glEnd());
//    glsafe(::glPopAttrib());
//
//    glsafe(::glPopMatrix());                // restore former projection matrix
//    glsafe(::glMatrixMode(GL_MODELVIEW));
//    glsafe(::glPopMatrix());                // restore former modelview matrix
//    glsafe(::glPopAttrib());                // restore former MatrixMode
//
//}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

} // namespace GUI
} // namespace Slic3r
