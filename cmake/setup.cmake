# Copyright © @CURRENT_YEAR@
string(TIMESTAMP CURRENT_YEAR "%Y")
# install-package.xml -> ReleaseDate
string(TIMESTAMP TODAY "%Y/%m/%d")

set(COMPANY_NAME "varian")
set(AUTHOR_NAME "varian")

# 如果需要生成exe安装包，需要配置qt binarycreator 路径
set(BINARY_CREATOR_PATH "D:\\qt\\Tools\\QtInstallerFramework\\4.10\\bin\\binarycreator.exe")