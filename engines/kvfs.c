#include "../fio.h"
#include "../optgroup.h"
#include "c.h"

static const char *nvme_options[] = {

};
static const int num_nvme_options =
    sizeof(nvme_options) / sizeof(nvme_options[0]);

static int fio_kvfs_setup(struct thread_data *td) {
  td->io_ops_data = kvfs_local_fs_open(nvme_options, num_nvme_options);
  return 0;
}

static int fio_kvfs_syncio_prep(struct thread_data *td, struct io_u *io_u) {
  io_u->file->engine_pos = 0;
  return 0;
}

static enum fio_q_status fio_syncio_queue(struct thread_data *td,
                                          struct io_u *io_u) {
  struct kvfs_file_t *f = io_u->file->engine_data;
  uint64_t offset = io_u->file->engine_pos;
  uint64_t n = 0;
  int ret = 0;
  fio_ro_check(td, io_u);
  if (io_u->ddir == DDIR_READ) {
    n = kvfs_file_read(f, io_u->xfer_buf, io_u->xfer_buflen, offset);
    ret = (n == io_u->xfer_buflen) ? 0 : -1;
  } else if (io_u->ddir == DDIR_WRITE) {
    n = kvfs_file_append(f, io_u->xfer_buf, io_u->xfer_buflen, offset);
    ret = (n == io_u->xfer_buflen) ? 0 : -1;
  } else {
    ret = kvfs_file_sync(f);
  }
  if (ret == 0) {
    io_u->file->engine_pos += n;
    io_u->error = 0;
  } else {
    io_u->error = errno;
  }
  return FIO_Q_COMPLETED;
}

static int fio_kvfs_open_file(struct thread_data *td, struct fio_file *f) {
  if (td_write(td) && td_read(td)) {
    f->engine_data = kvfs_open_file(td->io_ops_data, f->file_name);
    f->engine_pos = 0;
    return 0;
  }
  return -1;
}

static int fio_kvfs_close_file(struct thread_data *td, struct fio_file *f) {
  kvfs_close_file(f->engine_data);
  f->engine_pos = 0;
  return 0;
}

static int fio_kvfs_get_file_size(struct thread_data *td, struct fio_file *f) {
  return kvfs_file_size(f->engine_data);
}

static void fio_kvfs_cleanup(struct thread_data *td) {
  kvfs_local_fs_close(td->io_ops_data);
}

struct ioengine_ops ioengine = {
    .name = "kvfs",
    .version = FIO_IOOPS_VERSION,
    .setup = fio_kvfs_setup,
    .prep = fio_kvfs_syncio_prep,
    .queue = fio_syncio_queue,
    .open_file = fio_kvfs_open_file,
    .close_file = fio_kvfs_close_file,
    .get_file_size = fio_kvfs_get_file_size,
    .cleanup = fio_kvfs_cleanup,
    .flags = FIO_SYNCIO,
};

static void fio_init fio_nfs_register(void) { register_ioengine(&ioengine); }

static void fio_init fio_nfs_unregister(void) {
  unregister_ioengine(&ioengine);
}
