// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// Example graphics application, paired with uc2.C
// This demonstrates:
// - using shared memory to communicate with the worker app
// - reading XML preferences by which users can customize graphics
//   (in this case, select colors)
// - handle mouse input (in this case, to zoom and rotate)
// - draw text and 3D objects using OpenGL

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <math.h>
#endif
#include <string>
#include <vector>
#ifdef __APPLE__
#include "mac_app_icon.h"
#include "boinc_api.h"
#include <sys/socket.h>
#endif

#include "diagnostics.h"
#include "gutil.h"
#include "boinc_gl.h"
#include "graphics2.h"
#include "txf_util.h"
#include "network.h"
#include "gui_rpc_client.h"
#include "util.h"
#include "app_ipc.h"

using std::string;
using std::vector;

float white[4] = {1., 1., 1., 1.};
TEXTURE_DESC logo;
int width, height;      // window dimensions
bool mouse_down = false;
int mouse_x, mouse_y;
double pitch_angle, roll_angle, viewpoint_distance=10;
float color[4] = {.7, .2, .5, 1};
    // the color of the 3D object.
    // Can be changed using preferences

RPC_CLIENT rpc;
CC_STATE cc_state;
CC_STATUS cc_status;

#if 0
struct APP_SLIDES {
    string name;
    int index;
    double switch_time;
    vector<TEXTURE_DESC> slides;
    APP_SLIDES(string n): name(n), index(0), switch_time(0) {}
};

struct PROJECT_IMAGES {
    string url;
    TEXTURE_DESC icon;
    vector<APP_SLIDES> app_slides;
};

vector<PROJECT_IMAGES> project_images;
void icon_path(PROJECT* p, char* buf) {
    char dir[256];
    url_to_project_dir((char*)p->master_url.c_str(), dir);
    sprintf(buf, "%s/stat_icon", dir);
}

void slideshow(PROJECT* p) {
    char dir[256], buf[256];
    int i;

    url_to_project_dir((char*)p->master_url.c_str(), dir);
    for (i=0; i<99; i++) {
        sprintf(buf, "%s/slideshow_%02d", dir, i);
    }
}

PROJECT_IMAGES* get_project_images(PROJECT* p) {
    unsigned int i;
    char dir[256], path[256], filename[256];

    for (i=0; i<project_images.size(); i++) {
        PROJECT_IMAGES& pi = project_images[i];
        if (pi.url == p->master_url) return &pi;
    }
    PROJECT_IMAGES pim;
    pim.url = p->master_url;
    url_to_project_dir((char*)p->master_url.c_str(), dir);
    sprintf(path, "%s/stat_icon", dir);
    boinc_resolve_filename(path, filename, 256);
    pim.icon.load_image_file(filename);
    for (i=0; i<cc_state.apps.size(); i++) {
        APP& app = *cc_state.apps[i];
        if (app.project != p) continue;
        APP_SLIDES as(app.name);
        for (int j=0; j<99; j++) {
            sprintf(path, "%s/slideshow_%s_%02d", dir, app.name.c_str(), j);
            boinc_resolve_filename(path, filename, 256);
            TEXTURE_DESC td;
            int retval = td.load_image_file(filename);
            if (retval) break;
            as.slides.push_back(td);
        }
        pim.app_slides.push_back(as);
    }
    project_images.push_back(pim);
    return &(project_images.back());
}

#endif

// set up lighting model
//
static void init_lights() {
   GLfloat ambient[] = {1., 1., 1., 1.0};
   GLfloat position[] = {-13.0, 6.0, 20.0, 1.0};
   GLfloat dir[] = {-1, -.5, -3, 1.0};
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
}

static void draw_logo(float* pos, float alpha) {
    if (logo.present) {
        float size[3] = {.6, .4, 0};
        logo.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER, alpha);
    }
}

void show_result(RESULT* r, float x, float& y, float alpha) {
    PROGRESS_2D progress;
    char buf[256];
    txf_render_string(.1, x, y, 0, 1000., white, 0, (char*)r->project->project_name.c_str());
    y -= .02;
    float prog_pos[] = {x, y, 0};
    float prog_c[] = {.5, .4, .1, alpha/2};
    float prog_ci[] = {.1, .8, .2, alpha};
    progress.init(prog_pos, .4, -.01, -0.008, prog_c, prog_ci);
    progress.draw(r->fraction_done);
    sprintf(buf, "%.2f%% ", r->fraction_done*100);
    txf_render_string(.1, x+.41, y, 0, 1200., white, 0, buf);
    y -= .03;
    x += .05;
    sprintf(buf, "Elapsed: %.0f sec  Remaining: %.0f sec", r->elapsed_time, r->estimated_cpu_time_remaining);
    txf_render_string(.1, x, y, 0, 1200., white, 0, buf);
    y -= .03;
    sprintf(buf, "App: %s  Task: %s", (char*)r->app->user_friendly_name.c_str(),
        r->wup->name.c_str()
    );
    txf_render_string(.1, x, y, 0, 1200., white, 0, buf);
    y -= .03;
}

void show_coords() {
    int i;
    char buf[256];
    for (i=-100; i< 101; i+=5) {
        sprintf(buf, "%d", i);
        float x = (float)i/100;
        txf_render_string(.1, x, 0, 0, 1000., white, 0, buf);
    }
    for (i=-100; i< 101; i+=5) {
        sprintf(buf, "%d", i);
        float y = (float)i/100;
        txf_render_string(.1, 0, y, 0, 1000., white, 0, buf);
    }
}

void show_project(unsigned int index, float alpha) {
    float x=.2, y=.6;
    char buf[1024];
    if (!cc_state.projects.size()) {
        txf_render_string(.1, x, y, 0, 500., white, 0, "No projects attached");
        return;
    }
    txf_render_string(.1, x, y, 0, 1200., white, 0, "This computer is participating in");
    y -= .07;
    PROJECT *p = cc_state.projects[index];
    txf_render_string(.1, x, y, 0, 500., white, 0, (char*)p->project_name.c_str());
    y -= .07;
    txf_render_string(.1, x, y, 0, 800., white, 0, (char*)p->master_url.c_str());
    y -= .05;
    sprintf(buf, "User: %s", p->user_name.c_str());
    txf_render_string(.1, x, y, 0, 800., white, 0, buf);
    y -= .05;
    if (p->team_name.size()) {
        sprintf(buf, "Team: %s",  p->team_name.c_str());
        txf_render_string(.1, x, y, 0, 800., white, 0, buf);
        y -= .05;
    }
    sprintf(buf, "Total credit: %.0f   Average credit: %.0f", p->user_total_credit, p->user_expavg_credit);
    txf_render_string(.1, x, y, 0, 800., white, 0, buf);
    y -= .05;
    if (p->suspended_via_gui) {
        txf_render_string(.1, x, y, 0, 800., white, 0, "Suspended");
    }
}

void show_jobs(unsigned int index, double alpha) {
    float x=.1, y=.7;
    int nfound = 0;
    unsigned int i;
    for (i=0; i<cc_state.results.size(); i++) {
        RESULT* r = cc_state.results[i];
        if (!r->active_task) continue;
        if (r->active_task_state != PROCESS_EXECUTING) continue;
        if (nfound == index) {
            txf_render_string(.1, x, y, 0, 1200., white, 0, "Running tasks:");
            y -= .05;
            //x += .05;
        }
        if (nfound >= index && nfound < index+4) {
            show_result(r, x, y, alpha);
            y -= .05;
        }
        nfound++;
    }
    if (!nfound) {
        y = .5;
        txf_render_string(.1, x, y, 0, 500., white, 0, "No running tasks");
        char *p = 0;
        switch (cc_status.task_suspend_reason) {
        case SUSPEND_REASON_BATTERIES:
            p = "Computer is running on batteries"; break;
        case SUSPEND_REASON_USER_ACTIVE:
            p = "Computer is in use"; break;
        case SUSPEND_REASON_USER_REQ:
            p = "Computing suspended by user"; break;
        case SUSPEND_REASON_TIME_OF_DAY:
            p = "Computing suspended during this time of day"; break;
        case SUSPEND_REASON_BENCHMARKS:
            p = "Computing suspended while running benchmarks"; break;
        case SUSPEND_REASON_DISK_SIZE:
            p = "Computing suspended because no disk space"; break;
        case SUSPEND_REASON_NO_RECENT_INPUT:
            p = "Computing suspended while computer not in use"; break;
        case SUSPEND_REASON_INITIAL_DELAY:
            p = "Computing suspended while BOINC is starting up"; break;
        case SUSPEND_REASON_EXCLUSIVE_APP_RUNNING:
            p = "Computing suspended while exclusive application running"; break;
        }
        if (p) {
            y -= .1;
            txf_render_string(.1, x, y, 0, 800., white, 0, p);
        }
    }
}

void update_data() {
    int retval = rpc.get_state(cc_state);
    if (retval) {
        boinc_close_window_and_quit("RPC failed");
    }
    retval = rpc.get_cc_status(cc_status);
    if (retval) {
        boinc_close_window_and_quit("RPC failed");
    }
}

void set_viewpoint(double dist) {
    double x, y, z;
    x = 0;
    y = 3.0*dist;
    z = 11.0*dist;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        x, y, z,        // eye position
        0,-.8,0,        // where we're looking
        0.0, 1.0, 0.    // up is in positive Y direction
    );
    glRotated(pitch_angle, 1., 0., 0);
    glRotated(roll_angle, 0., 1., 0);
}

static void init_camera(double dist) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
        45.0,       // field of view in degree
        1.0,        // aspect ratio
        1.0,        // Z near clip
        1000.0      // Z far
    );
    set_viewpoint(dist);
}

struct FADER {
    double grow, on, fade, off;
    double start, total;
    FADER(double g, double n, double f, double o) {
        grow = g;
        on = n;
        fade = f;
        off = o;
        start = 0;
        total = grow + on + fade + off;
    }
    bool value(double t, double& v) {
        if (!start) {
            start = t;
            v = 0;
            return false;
        }
        double dt = t - start;
        if (dt > total) {
            start = t;
            v = 0;
            return true;
        }
        if (dt < grow) {
            v = dt/grow;
        } else if (dt < grow+on) {
            v = 1;
        } else if (dt < grow + on + fade) {
            double x = dt-(grow+on);
            v = 1-(x/fade);
        } else {
            v = 0;
        }
        return false;
    }
};

FADER logo_fader(5,5,5,2);
FADER info_fader(4,4,4,1);

void app_graphics_render(int xs, int ys, double t) {
    double alpha;
    static bool showing_project = false;
    static unsigned int project_index = 0, job_index=0;
    static float logo_pos[3] = {.2, .2, 0};
    static bool first = true;

    if (first) {
        update_data();
        first = false;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw logo first - it's in background
    //
    mode_unshaded();
    mode_ortho();
    if (logo_fader.value(t, alpha)) {
        logo_pos[0] = drand()*.4;
        logo_pos[1] = drand()*.4;
    }
    draw_logo(logo_pos, (float)alpha);
    //ortho_done();

    // draw 3D objects
    //
    //init_camera(viewpoint_distance);
    //scale_screen(width, height);
    //mode_shaded(color);

    // draw text on top
    //
    //mode_unshaded();
    //mode_ortho();

    if (info_fader.value(t, alpha)) {
        update_data();
        if (showing_project) {
            showing_project = false;
            project_index++;
        } else {
            job_index += 4;
            if (job_index >= cc_state.results.size()) {
                job_index = 0;
            }
            showing_project = true;
        }
    }
    white[3] = alpha;
    if (showing_project) {
        if (project_index >= cc_state.projects.size()) {
            project_index = 0;
        }
        show_project(project_index, alpha);
    } else {
        show_jobs(job_index, alpha);
    }
    //show_coords();
    ortho_done();
}

void app_graphics_resize(int w, int h){
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}

// mouse drag w/ left button rotates 3D objects;
// mouse draw w/ right button zooms 3D objects
//
void boinc_app_mouse_move(int x, int y, int left, int middle, int right) {
    if (left) {
        pitch_angle += (y-mouse_y)*.1;
        roll_angle += (x-mouse_x)*.1;
        mouse_y = y;
        mouse_x = x;
    } else if (right) {
        double d = (y-mouse_y);
        viewpoint_distance *= exp(d/100.);
        mouse_y = y;
        mouse_x = x;
    } else {
        mouse_down = false;
    }
}

void boinc_app_mouse_button(int x, int y, int which, int is_down) {
    if (is_down) {
        mouse_down = true;
        mouse_x = x;
        mouse_y = y;
    } else {
        mouse_down = false;
    }
}

void boinc_app_key_press(int, int){}

void boinc_app_key_release(int, int){}

void app_graphics_init() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    txf_load_fonts(".");
    logo.load_image_file("boinc_logo_black.jpg");
    init_lights();
}

int main(int argc, char** argv) {
    int retval;

#ifdef _WIN32
    WinsockInitialize();
#endif
    retval = rpc.init("localhost");
    if (retval) exit(retval);

#ifdef __APPLE__
    setMacIcon(argv[0], MacAppIconData, sizeof(MacAppIconData));
#endif
    boinc_graphics_loop(argc, argv);
    boinc_finish_diag();
#ifdef _WIN32
    WinsockCleanup();
#endif
}
