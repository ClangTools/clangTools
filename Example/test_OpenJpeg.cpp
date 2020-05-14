//
// Created by caesar kekxv on 2020/4/19.
//

#include <cstdio>
#include <string>
#include <getopt.h>
#include <c_vector.h>
#include <openjpeg-2.3/openjpeg.h>
#include <openjpeg-2.3/opj_stdint.h>
#include <openjpeg/color.h>

using namespace std;

int main(int argc, char *argv[]) {
    c_vector *infile = nullptr, *outfile = nullptr;
    FILE *file = nullptr;
    int64_t _size;
    size_t size;
    string in_path;
    string out_path;
    uint8_t *buf = nullptr;
    uint32_t decod_format = OPJ_CODEC_J2K;

    opj_image_t *image = nullptr;
    opj_stream_t *l_stream = nullptr;              /* Stream */
    opj_codec_t *l_codec = nullptr;                /* Handle to a decompressor */
    opj_codestream_index_t *cstr_index = nullptr;
    opj_dparameters_t parameters_core;
    /* default decoding parameters (core) */
    opj_set_default_decoder_parameters(&(parameters_core));


    int ch;
    c_vector_init(&infile);
    c_vector_init(&outfile);
    opterr = 0;
    while ((ch = getopt(argc, argv, "i:o:t:")) != -1) {
        switch (ch) {
            case 'i':
                in_path = optarg;
                break;
            case 'o':
                out_path = optarg;
                break;
            case 't':
                decod_format = strtol(optarg, nullptr,10);
                break;
            default:
                printf("other option :%c\n", ch);
                break;
        }
    }
    if (in_path.empty() || out_path.empty() || infile == nullptr || outfile == nullptr) {
        goto fin;
    }
    file = fopen(in_path.c_str(), "rb");
    if (file == nullptr) {
        goto fin;
    }
    fseek(file, 0, SEEK_SET);
    fseek(file, 0, SEEK_END);
    _size = ftello(file);
    if (_size < 0)size = -_size;
    else size = _size;
    buf = new uint8_t[size];
    fread(buf, 1, size, file);
    c_vector_push_back(infile, buf, 0, size);
    delete[] buf;
    buf = nullptr;

    l_stream = opj_stream_create_default_c_vector(infile, 1);
    if (!l_stream) {
        goto fin;
    }
    /* decode the JPEG2000 stream */
    /* ---------------------- */

    switch (decod_format) {
        case OPJ_CODEC_J2K: { /* JPEG-2000 codestream */
            /* Get a decoder handle */
            l_codec = opj_create_decompress(OPJ_CODEC_J2K);
            break;
        }
        case OPJ_CODEC_JP2: { /* JPEG 2000 compressed image data */
            /* Get a decoder handle */
            l_codec = opj_create_decompress(OPJ_CODEC_JP2);
            break;
        }
        case OPJ_CODEC_JPT: { /* JPEG 2000, JPIP */
            /* Get a decoder handle */
            l_codec = opj_create_decompress(OPJ_CODEC_JPT);
            break;
        }
        default:
            opj_stream_destroy(l_stream);
            goto fin;
    }
    /* Setup the decoder decoding parameters using user parameters */
    if (!opj_setup_decoder(l_codec, &parameters_core)) {
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        goto fin;
    }
    /* Read the main header of the codestream and if necessary the JP2 boxes*/
    if (!opj_read_header(l_stream, l_codec, &image)) {
        opj_stream_destroy(l_stream);
        opj_destroy_codec(l_codec);
        opj_image_destroy(image);
        goto fin;
    }
    {
        if (!opj_get_decoded_tile(l_codec, l_stream, image, 0)) {
            opj_destroy_codec(l_codec);
            opj_stream_destroy(l_stream);
            opj_image_destroy(image);
            goto fin;
        }
    }


    /* Close the byte stream */
    opj_stream_destroy(l_stream);

    if (image->color_space != OPJ_CLRSPC_SYCC
        && image->numcomps == 3 && image->comps[0].dx == image->comps[0].dy
        && image->comps[1].dx != 1) {
        image->color_space = OPJ_CLRSPC_SYCC;
    } else if (image->numcomps <= 2) {
        image->color_space = OPJ_CLRSPC_GRAY;
    }

    if (image->color_space == OPJ_CLRSPC_SYCC) {
        color_sycc_to_rgb(image);
    } else if (image->color_space == OPJ_CLRSPC_CMYK) {
        color_cmyk_to_rgb(image);
    } else if (image->color_space == OPJ_CLRSPC_EYCC) {
        color_esycc_to_rgb(image);
    }

    if (image->icc_profile_buf) {
#if defined(OPJ_HAVE_LIBLCMS1) || defined(OPJ_HAVE_LIBLCMS2)
        if (image->icc_profile_len) {
                color_apply_icc_profile(image);
            } else {
                color_cielab_to_rgb(image);
            }
#endif
        free(image->icc_profile_buf);
        image->icc_profile_buf = NULL;
        image->icc_profile_len = 0;
    }



    fin:
    c_vector_free(&infile);
    c_vector_free(&outfile);
    return 0;
}