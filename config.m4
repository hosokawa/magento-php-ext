PHP_ARG_ENABLE(mage_path, whether to enable magento path finder extension,
[ --enable-mage-path   Enable magento path finder extension])
 
if test "$PHP_MAGE_PATH" = "yes"; then
  AC_DEFINE(HAVE_MAGE_PATH, 1, [Whether you have magento path finder extension])
  PHP_NEW_EXTENSION(mage_path, mage_path.c, $ext_shared)
fi
