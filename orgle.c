#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
MODE_ORG,
MODE_CODE,
MODE_BEGINCODE
};

enum {
SEGTYPE_TEXT,
SEGTYPE_REFERENCE
};

typedef struct orgle_block orgle_block;

typedef struct {
    char *str;
    size_t size;
} orgle_string;

typedef struct orgle_segment {
    int type;
    orgle_string str;
    size_t linum;
    orgle_string *filename;
    struct orgle_segment *nxt;
} orgle_segment;

typedef struct orgle_file {
    orgle_string filename;
    orgle_block *top;
    struct orgle_file *nxt;
} orgle_file;

struct orgle_block {
    int nsegs;
    orgle_segment *head;
    orgle_segment *tail;
    orgle_string name;
    struct orgle_block *nxt;
};

typedef struct {
    int nblocks;
    orgle_block *head;
    orgle_block *tail;
} orgle_blocklist;

typedef struct {
    orgle_blocklist blk[256];
    int nwords;
} orgle_hashmap;

typedef struct {
    orgle_file *head;
    orgle_file *tail;
    int nfiles;
} orgle_filelist;

typedef struct {
    orgle_string block; /* TODO: rename */
    size_t curline;
    int block_started; /* TODO: rename */
    orgle_hashmap dict;
    orgle_filelist flist;
    char *buf;
    orgle_block *curblock;
    size_t linum;
    orgle_string filename;
} orgle_d;

int orgle_hashmap_find(orgle_hashmap *h, orgle_string *name, orgle_block **b);
int orgle_block_write(orgle_block *b, orgle_hashmap *h, FILE *fp);

static int orgle_string_write(FILE *fp, orgle_string *str)
{
    return fwrite(str->str, 1, str->size, fp);
}

static void orgle_string_reset(orgle_string *str)
{
    str->str = NULL;
    str->size = 0;
}

static void orgle_string_init(orgle_string *str)
{
    orgle_string_reset(str);
}

static int parse_begin(char *line, size_t len, orgle_string *str)
{
    size_t n;
    int mode;
    int rc;

    line += 11;
    len -= 11;

    if(len <= 0) return 0;


    mode = 0;
    n = 0;
    rc = 1;
    str->str = NULL;
    str->size = 0;
    while(n < len) {
        switch(mode) {
            case 0:
                if(line[n] == ' ') {
                    n++;
                } else {
                    mode = 1;
                }
                break;
            case 1:
                if(line[n] == ' ') {
                    mode = 0;
                    n++;
                } else {
                    if(line[n] == ':') {
                        if(!strncmp(line + n + 1, "tangle", 6)) {
                            n+=7;
                            mode = 2;
                            rc = 2;
                        }
                    }
                    n++;
                }
                break;
            case 2: /* spaces after tangle */
                if(line[n] != ' ') {
                    str->str = &line[n];
                    str->size++;
                    mode = 3;
                }
                n++;
                break;
            case 3: /* read up to next space or line break */
                if(line[n] == ' ' || line[n] == '\n') {
                    mode = 4;
                } else {
                    str->size++;
                }
                n++;
                break;
            case 4: /* countdown til end */
                n++;
                break;
        }
    }

    return rc;
}

static int parse_name(char *line, size_t len, orgle_string *str)
{
    size_t n;
    size_t pos;
    int mode;

    line+=7;
    len-=7;
    /* *namelen = 0; */
    str->size = 0;
    str->str = NULL;
    if(len <= 0) return 1;
    pos = 0;
    mode = 0;
    for(n = 0; n < len; n++) {
        if(mode == 2) break;
        switch(mode) {
            case 0:
                if(line[n] == ' ') {

                } else {
                    /* buf[pos] = line[n]; */
                    str->str = &line[n];
                    str->size++;
                    pos++;
                    mode = 1;
                }
                break;
            case 1:
                if(line[n] == 0xa) {
                    mode = 2;
                    /* buf[pos] = 0; */
                    break;
                }
                /* buf[pos] = line[n]; */
                pos++;
                str->size++;
                break;
            default:
                break;
        }
    }
    /* *namelen = pos; */
    return 1;
}

static int orgle_getline(char *fullbuf,
                  char **buf,
                  size_t *pos,
                  size_t *line_size,
                  size_t buf_size)
{
    size_t p;
    size_t s;
    *line_size = 0;
    p = *pos;
    *buf = &fullbuf[p];
    s = 0;
    while(1) {
        s++;
        if(p >= buf_size) return 0;
        if(fullbuf[p] == '\n') {
            *pos = p + 1;
            *line_size = s;
            return 1;
        }
        p++;
    }
}

static int check_for_reference(char *line , size_t size, orgle_string *str)
{
    int mode;
    size_t n;
    mode = 0;

    str->size = 0;
    str->str = NULL;
    for(n = 0; n < size; n++) {
        if(mode < 0) break;
        switch(mode) {
            case 0: /* spaces */
                if(line[n] == ' ') continue;
                else if(line[n] == '<') mode = 1;
                else mode = -1;
                break;
            case 1: /* second < */
                if(line[n] == '<') mode = 2;
                else mode = -1;
                break;
            case 2: /* word setup */
                str->str = &line[n];
                str->size++;
                mode = 3;
                break;
            case 3: /* the word */
                if(line[n] == '>') {
                    mode = 4;
                    break;
                }
                str->size++;
                break;
            case 4: /* last > */
                if(line[n] == '>') mode = 5;
                else mode = -1;
                break;
        }
    }

    return (mode == 5);
}

void orgle_block_init(orgle_block *b)
{
    b->nsegs = 0;
    b->head = NULL;
    b->tail = NULL;
    b->nxt = NULL;
    orgle_string_init(&b->name);
}

void orgle_block_free(orgle_block *lst)
{
    orgle_segment *s;
    orgle_segment *nxt;
    int n;
    s = lst->head;
    for(n = 0; n < lst->nsegs; n++) {
        nxt = s->nxt;
        free(s);
        s = nxt;
    }
}

void orgle_block_append_segment(orgle_block *b,
                                orgle_string *str,
                                int type,
                                size_t linum,
                                orgle_string *filename)
{
    orgle_segment *s;
    s = malloc(sizeof(orgle_segment));
    s->str = *str;
    s->type = type;
    if(b->nsegs == 0) {
        b->head = s;
        b->tail = s;
    }
    s->linum = linum;
    s->filename = filename;
    b->tail->nxt = s;
    b->tail = s;
    b->nsegs++;
}

void orgle_block_append_string(orgle_block *b,
                               orgle_string *str,
                               size_t linum,
                               orgle_string *filename)
{
    orgle_block_append_segment(b, str, SEGTYPE_TEXT, linum, filename);
}

void orgle_block_append_reference(orgle_block *b,
                                  orgle_string *str,
                                  size_t linum,
                                  orgle_string *filename)
{
    orgle_block_append_segment(b, str, SEGTYPE_REFERENCE, linum, filename);
}

int orgle_segment_write(orgle_segment *s, orgle_hashmap *h, FILE *fp)
{
    orgle_block *b;
    fprintf(fp, "#line %lu \"", s->linum);
    orgle_string_write(fp, s->filename);
    fprintf(fp, "\"\n");
    if(s->type == SEGTYPE_TEXT) {
        orgle_string_write(fp, &s->str);
    } else {
        if(!orgle_hashmap_find(h, &s->str, &b)) {
            fprintf(stderr, "Could not find reference segment '");
            orgle_string_write(stderr, &s->str);
            fprintf(stderr, "'\n");
            return 0;
        }
        return orgle_block_write(b, h, fp);
    }

    return 1;
}

int orgle_block_write(orgle_block *b, orgle_hashmap *h, FILE *fp)
{
    orgle_segment *s;
    int n;
    s = b->head;
    for(n = 0; n < b->nsegs; n++) {
        if(!orgle_segment_write(s, h, fp)) return 0;
        s = s->nxt;
    }

    return 1;
}

void orgle_blocklist_init(orgle_blocklist *lst)
{
    lst->head = NULL;
    lst->tail = NULL;
    lst->nblocks = 0;
}

void orgle_blocklist_free(orgle_blocklist *lst)
{
    orgle_block *b;
    orgle_block *nxt;
    int n;
    b = lst->head;
    for(n = 0; n < lst->nblocks; n++) {
        nxt = b->nxt;
        orgle_block_free(b);
        free(b);
        b = nxt;
    }
}

void orgle_blocklist_append(orgle_blocklist *lst, orgle_block *b)
{
    if(lst->nblocks == 0) {
        lst->head = b;
        lst->tail = b;
    }
    lst->tail->nxt = b;
    lst->tail = b;
    lst->nblocks++;
}

void orgle_hashmap_init(orgle_hashmap *h)
{
    int n;
    h->nwords = 0;
    for(n = 0; n < 256; n++) {
        orgle_blocklist_init(&h->blk[n]);
    }
}

void orgle_hashmap_free(orgle_hashmap *h)
{
    int n;
    for(n = 0; n < 256; n++) {
        orgle_blocklist_free(&h->blk[n]);
    }
}

static int hash(const char *str, size_t size)
{
    unsigned int h = 5381;
    size_t i = 0;

    for(i = 0; i < size; i++) {
        h = ((h << 5) + h) ^ str[i];
        h %= 0x7FFFFFFF;
    }

    return h % 256;
}

int orgle_hashmap_find(orgle_hashmap *h, orgle_string *name, orgle_block **b)
{
    int pos;
    orgle_blocklist *lst;
    int n;
    orgle_block *blk;
    pos = hash(name->str, name->size);
    lst = &h->blk[pos];

    blk = lst->head;
    for(n = 0; n < lst->nblocks; n++) {
        if(name->size == blk->name.size) {
            if(!strncmp(name->str, blk->name.str, name->size)) {
                *b = blk;
                return 1;
            }
        }
        blk = blk->nxt;
    }
    return 0;
}

orgle_block * orgle_hashmap_get(orgle_hashmap *h, orgle_string *name)
{
    orgle_block *b;
    orgle_blocklist *lst;
    int pos;

    if(orgle_hashmap_find(h, name, &b)) return b;
    pos = hash(name->str, name->size);
    b = NULL;
    b = malloc(sizeof(orgle_block));
    orgle_block_init(b);
    b->name = *name;
    lst = &h->blk[pos];
    orgle_blocklist_append(lst, b);
    return b;
}

void orgle_filelist_init(orgle_filelist *flist)
{
    flist->head = NULL;
    flist->tail = NULL;
    flist->nfiles = 0;
}

void orgle_filelist_free(orgle_filelist *flist)
{
    orgle_file *f;
    orgle_file *nxt;
    int n;
    f = flist->head;
    for(n = 0; n < flist->nfiles; n++) {
        nxt = f->nxt;
        free(f);
        f = nxt;
    }
}

void orgle_filelist_append(orgle_filelist *flist,
                           orgle_string *name,
                           orgle_block *top)
{
    orgle_file *f;
    f = malloc(sizeof(orgle_file));
    f->filename = *name;
    f->top = top;

    if(flist->nfiles == 0) {
        flist->head = f;
        flist->tail = f;
    }
    flist->tail->nxt = f;
    flist->tail = f;
    flist->nfiles++;
}

int orgle_file_write(orgle_file *f, orgle_hashmap *h)
{
    FILE *fp;
    char tmp[128];
    size_t n;
    size_t size;
    int rc;

    if(f->filename.size > 128) size = 127;
    else size = f->filename.size;
    for(n = 0; n < size; n++) tmp[n] = f->filename.str[n];
    tmp[size] = 0;

    fp = fopen(tmp, "w");

    rc = orgle_block_write(f->top, h, fp);

    fclose(fp);
    return rc;
}

int orgle_filelist_write(orgle_filelist *flist, orgle_hashmap *h)
{
    orgle_file *f;
    int n;

    f = flist->head;
    for(n = 0; n < flist->nfiles; n++) {
        if(!orgle_file_write(f, h)) return 0;
        f = f->nxt;
    }

    return 1;
}

void orgle_init(orgle_d *org)
{
    org->block_started = 0;
    org->buf = NULL;
    orgle_hashmap_init(&org->dict);
    org->curblock = NULL;
    orgle_filelist_init(&org->flist);
    org->linum = 0;
    orgle_string_init(&org->filename);
    org->curline = -1;
}

int orgle_parse_filename(orgle_d *org, const char *filename)
{
    /* TODO: implement */
    return 1;
}

void orgle_free(orgle_d *org)
{
    if(org->buf != NULL) free(org->buf);
    orgle_hashmap_free(&org->dict);
    orgle_filelist_free(&org->flist);
}

void orgle_begin_block(orgle_d *org, orgle_string *name)
{
    org->curblock = orgle_hashmap_get(&org->dict, name);
}

void orgle_append_string(orgle_d *org)
{
    if(org->curblock == NULL) return;
    orgle_block_append_string(org->curblock,
                              &org->block,
                              org->curline,
                              &org->filename);
}

void orgle_append_reference(orgle_d *org, orgle_string *ref)
{
    if(org->curblock == NULL) return;
    orgle_block_append_reference(org->curblock,
                                 ref,
                                 org->linum,
                                 &org->filename);
}

void orgle_append_file(orgle_d *org, orgle_string *filename)
{
    orgle_filelist_append(&org->flist, filename, org->curblock);
}

int orgle_generate(orgle_d *org)
{
    return orgle_filelist_write(&org->flist, &org->dict);
}

int main(int argc, char *argv[])
{
    FILE *fp;
    char *line;
    size_t read;

    int mode;
    int rc;
    char *buf;
    size_t size;
    int status;
    size_t pos;
    orgle_d org;
    orgle_string str;

    line = NULL;

    mode = MODE_ORG;

    rc = 0;

    orgle_init(&org);

    if(argc < 2) {
        fprintf(stderr, "Usage: %s filename.org\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "r");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);

    buf = calloc(1, size);
    org.buf = buf;

    fseek(fp, 0, SEEK_SET);
    fread(buf, size, 1, fp);
    fclose(fp);

    org.filename.str = argv[1];
    org.filename.size = strlen(argv[1]);

    pos = 0;
    while(1) {
        org.linum++;
        status = orgle_getline(buf, &line, &pos, &read, size);
        if(!status) break;
        if(mode == MODE_ORG) {
            if(read >= 7) {
                if(!strncmp(line, "#+NAME:",7)) {
                    mode = MODE_BEGINCODE;
                    parse_name(line, read, &str);
                    orgle_begin_block(&org, &str);
                }
            }
        } else if(mode == MODE_CODE) {
            if(read >= 9) {
                if(!strncmp(line, "#+END_SRC", 9)) {
                    mode = MODE_ORG;
                    org.block_started = 0;
                    orgle_append_string(&org);
                    continue;
                }
            }

            if(check_for_reference(line, read, &str)) {
                orgle_append_string(&org);
                orgle_append_reference(&org, &str);
                org.block_started = 1;
                orgle_string_reset(&org.block);
                continue;
            }

            org.block.size += read;

            if(org.block_started) {
                org.block.str = line;
                org.block_started = 0;
                org.curline = org.linum;
            }
        } else if (mode == MODE_BEGINCODE) {
            if(read >= 11) {
                if(!strncmp(line, "#+BEGIN_SRC",11)) {
                    mode = MODE_CODE;
                    org.block_started = 1;
                    orgle_string_reset(&org.block);
                    if(parse_begin(line, read, &str) == 2) {
                        orgle_append_file(&org, &str);
                    }
                    continue;
                } else {
                    fwrite(line, read, 1, stderr);
                    fprintf(stderr, "line %lu: Expected #+BEGIN_SRC\n", org.linum);
                    rc = 1;
                    break;
                }
            }
            fprintf(stderr, "line %lu: Expected #+BEGIN_SRC\n", org.linum);
            rc = 1;
        }
    }

    if(!rc) if(!orgle_generate(&org)) rc = 1;

    orgle_free(&org);
    return rc;
}
