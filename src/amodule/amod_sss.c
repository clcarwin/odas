
    #include "amod_sss.h"

    amod_sss_obj * amod_sss_construct(const mod_sss_cfg * mod_sss_config, const msg_spectra_cfg * msg_spectra_config, const msg_tracks_cfg * msg_tracks_config) {

        amod_sss_obj * obj;

        obj = (amod_sss_obj *) malloc(sizeof(amod_sss_obj));

        obj->mod_sss = mod_sss_construct(mod_sss_config, msg_spectra_config, msg_tracks_config);

        obj->in1 = (amsg_spectra_obj *) NULL;
        obj->in2 = (amsg_tracks_obj *) NULL;
        obj->out = (amsg_spectra_obj *) NULL;
        
        obj->thread = thread_construct(&amod_sss_thread, (void *) obj);

        return obj;

    }

    void amod_sss_destroy(amod_sss_obj * obj) {

        mod_sss_destroy(obj->mod_sss);
        thread_destroy(obj->thread);

        free((void *) obj); 

    }

    void amod_sss_connect(amod_sss_obj * obj, amsg_spectra_obj * in1, amsg_tracks_obj * in2, amsg_spectra_obj * out) {

        obj->in1 = in1;
        obj->in2 = in2;
        obj->out = out;

    }

    void amod_sss_disconnect(amod_sss_obj * obj) {

        obj->in1 = (amsg_spectra_obj *) NULL;
        obj->in2 = (amsg_tracks_obj *) NULL;
        obj->out = (amsg_spectra_obj *) NULL;

    }

    void * amod_sss_thread(void * ptr) {

        amod_sss_obj * obj;
        msg_spectra_obj * msg_spectra_in;
        msg_tracks_obj * msg_tracks_in;
        msg_spectra_obj * msg_spectra_out;
        int rtnValue;

        obj = (amod_sss_obj *) ptr;

        while(1) {

            // Pop a message, process, and push back
            msg_spectra_in = amsg_spectra_filled_pop(obj->in1);
            msg_tracks_in = amsg_tracks_filled_pop(obj->in2);
            msg_spectra_out = amsg_spectra_empty_pop(obj->out);
            mod_sss_connect(obj->mod_sss, msg_spectra_in, msg_tracks_in, msg_spectra_out);
            rtnValue = mod_sss_process(obj->mod_sss);
            mod_sss_disconnect(obj->mod_sss);
            amsg_spectra_empty_push(obj->in1, msg_spectra_in);
            amsg_tracks_empty_push(obj->in2, msg_tracks_in);
            amsg_spectra_filled_push(obj->out, msg_spectra_out);

            // If this is the last frame, rtnValue = -1
            if (rtnValue == -1) {
                break;
            }

        }

    }