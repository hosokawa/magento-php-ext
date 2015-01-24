#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"

#define PHP_MAGE_PATH_VERSION "1.0"
#define PHP_MAGE_PATH_EXTNAME "mage_path"
 
extern zend_module_entry mage_path_module_entry;
#define phpext_mage_path_ptr &mage_path_module_entry
 
// declaration of a custom my_function()
PHP_FUNCTION(mage_path_open);
PHP_FUNCTION(mage_path_get_skin);
PHP_FUNCTION(mage_path_get_layout);
PHP_FUNCTION(mage_path_get_template);
PHP_FUNCTION(mage_path_close);

 
// list of custom PHP functions provided by this extension
// set {NULL, NULL, NULL} as the last record to mark the end of list
static zend_function_entry mage_path_functions[] = {
    PHP_FE(mage_path_open, NULL)
    PHP_FE(mage_path_get_skin, NULL)
    PHP_FE(mage_path_get_layout, NULL)
    PHP_FE(mage_path_get_template, NULL)
    PHP_FE(mage_path_close, NULL)
    {NULL, NULL, NULL}
};
 
// the following code creates an entry for the module and registers it with Zend.
zend_module_entry mage_path_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_MAGE_PATH_EXTNAME,
    mage_path_functions,
    NULL, // name of the MINIT function or NULL if not applicable
    NULL, // name of the MSHUTDOWN function or NULL if not applicable
    NULL, // name of the RINIT function or NULL if not applicable
    NULL, // name of the RSHUTDOWN function or NULL if not applicable
    NULL, // name of the MINFO function or NULL if not applicable
#if ZEND_MODULE_API_NO >= 20010901
    PHP_MAGE_PATH_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};
 
ZEND_GET_MODULE(mage_path)
 
typedef struct s_mage_path {
    int num_array;
    int append_max;
    char *base;
    char *base_url;
    char *buf;
    char **path_array;
} MAGE_PATH;
typedef MAGE_PATH *H_MAGE_PATH;
void mage_path_free(H_MAGE_PATH h);
int mage_path_exists(const char *path);

PHP_FUNCTION(mage_path_open) {
    int base_len;
    int base_url_len;
    char *base;
    char *base_url;
    zval *array;
    zval **data;
    HashTable *arr_hash;
    int arr_count;
    HashPosition pointer;
    H_MAGE_PATH h;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssa", &base, &base_len, &base_url, &base_url_len, &array) == FAILURE) {
        RETURN_NULL();
    }
    if (Z_TYPE_P(array) != IS_ARRAY) {
        RETURN_NULL();
    }

    arr_hash = Z_ARRVAL_P(array);
    arr_count = 0;
    for (
        zend_hash_internal_pointer_reset_ex(arr_hash, &pointer);
        zend_hash_get_current_data_ex(arr_hash, (void**)&data, &pointer) == SUCCESS;
        zend_hash_move_forward_ex(arr_hash, &pointer)
    ) {
        if (Z_TYPE_PP(data) == IS_STRING) {
            arr_count++;
        }
    }
    if (arr_count == 0) {
        RETURN_NULL();
    }

    h = calloc(1, sizeof(MAGE_PATH));
    if (h) {
        h->num_array = arr_count;
        h->path_array = calloc(arr_count, sizeof(char*));
        if (h->path_array) {
            int i = 0;
            int max_len = 0;
            for (
                zend_hash_internal_pointer_reset_ex(arr_hash, &pointer);
                zend_hash_get_current_data_ex(arr_hash, (void**)&data, &pointer) == SUCCESS;
                zend_hash_move_forward_ex(arr_hash, &pointer)
            ) {
                if (Z_TYPE_PP(data) == IS_STRING) {
                    h->path_array[i] = malloc(Z_STRLEN_PP(data) + sizeof(char));
                    if (h->path_array[i]) {
                        strncpy(h->path_array[i], Z_STRVAL_PP(data), Z_STRLEN_PP(data));
                        h->path_array[i][Z_STRLEN_PP(data)] = '\0';
                        if (max_len < Z_STRLEN_PP(data)) {
                            max_len = Z_STRLEN_PP(data);
                        }
                    } else {
                        for (int j = i - 1; j >= 0; j--) {
                            free(h->path_array[j]);
                        }
                        free(h->path_array);
                        free(h);
                        RETURN_NULL();
                    }
                    i++;
                }
            }
            if (max_len > 0) {
                // TODO なんか間違ってるような気がする。
                h->append_max = 2048 - 1 - max_len - 30; // 1:'\0', 30:最大の固定文字列部分、 'app/design/frontend/..max_len../template/..append_max..'
                h->buf = malloc(2048);
            }
            if (h->buf) {
                h->base = malloc(base_len + 1);
                if (h->base) {
                    strncpy(h->base, base, base_len);
                    h->base[base_len] = '\0';
                    h->base_url = malloc(base_url_len + 1);
                    if (h->base_url) {
                        strncpy(h->base_url, base_url, base_url_len);
                        h->base_url[base_url_len] = '\0';
                        RETURN_LONG((long)h);
                    } else {
                        mage_path_free(h);
                    }
                } else {
                    mage_path_free(h);
                }
            } else {
                mage_path_free(h);
            }
        } else {
            free(h);
        }
    }
    RETURN_NULL();
}

void path_to_url(H_MAGE_PATH h) {
}

PHP_FUNCTION(mage_path_get_skin) {
    H_MAGE_PATH h;
    long get_h;
    char *path;
    int path_len;
    int len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &get_h, &path, &path_len) == FAILURE) {
        RETURN_NULL();
    }

    h = (H_MAGE_PATH)get_h;
    if (path_len >= h->append_max) {
        RETURN_NULL();
    }
    for (int i = 0; i < h->num_array; i++) {
        sprintf(h->buf, "%s/skin/frontend/%s/", h->base, h->path_array[i]);
        len = strlen(h->buf);
        strncat(h->buf, path, path_len);
        len += path_len;
        h->buf[len] = '\0';

        if (mage_path_exists(h->buf)) {
            sprintf(h->buf, "%s/skin/frontend/%s/", h->base_url, h->path_array[i]);
            len = strlen(h->buf);
            strncat(h->buf, path, path_len);
            len += path_len;
            h->buf[len] = '\0';
            RETURN_STRING(h->buf, 1);
        }
    }
    sprintf(h->buf, "%s/skin/frontend/%s/", h->base_url, h->path_array[h->num_array - 1]);
    len = strlen(h->buf);
    strncat(h->buf, path, path_len);
    len += path_len;
    h->buf[len] = '\0';
    RETURN_STRING(h->buf, 1);
}

PHP_FUNCTION(mage_path_get_layout) {
    H_MAGE_PATH h;
    long get_h;
    char *path;
    int path_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &get_h, &path, &path_len) == FAILURE) {
        RETURN_NULL();
    }
    h = (H_MAGE_PATH)get_h;
}

PHP_FUNCTION(mage_path_get_template) {
    H_MAGE_PATH h;
    long get_h;
    char *path;
    int path_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &get_h, &path, &path_len) == FAILURE) {
        RETURN_NULL();
    }
    h = (H_MAGE_PATH)get_h;
}

int mage_path_exists(const char *path) {
    struct stat check;
    int ret = stat(path, &check);
    if (ret == 0) {
        return 1;
    }
    return 0;
}

void mage_path_free(H_MAGE_PATH h) {
    for (int i = 0; i < h->num_array; i++) {
        free(h->path_array[i]);
    }
    free(h->path_array);
    free(h->base);
    free(h->base_url);
    free(h->buf);
    free(h);
}

PHP_FUNCTION(mage_path_close) {
    H_MAGE_PATH h;
    long get_h;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &get_h) == FAILURE) {
        RETURN_NULL();
    }
    h = (H_MAGE_PATH)get_h;
    mage_path_free(h);
}
