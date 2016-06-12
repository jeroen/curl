FROM rocker/r-devel-san

RUN apt-get update -qq \
	&& apt-get dist-upgrade -y \
	&& apt-get install pandoc libxml2-dev -y

RUN RD -e 'install.packages("curl", depends = TRUE)' \
	&& RD -e 'install.packages("https://github.com/jeroenooms/curl/archive/master.tar.gz", repos = NULL, INSTALL_opts = "--install-tests")'

RUN RD -e 'testthat::test_package("curl")'
