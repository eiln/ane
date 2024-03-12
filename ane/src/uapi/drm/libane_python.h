#ifdef CONFIG_DRM_ACCEL && CONFIG_DRM_ACCEL_ANE
#define __PYANE_H__
void *pyane_init(char *path, int dev_id);
int pyane_free(struct ane_nn *nn);
int pyane_exec(struct ane_nn *nn);
int pyane_send(struct ane_nn *nn, void **head_pyane_tensor);
int pyane_read(struct ane_nn *nn, void **head_pyane_tensor);
