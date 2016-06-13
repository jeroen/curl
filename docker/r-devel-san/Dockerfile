FROM rocker/r-devel-san

RUN apt-get update -qq \
	&& apt-get dist-upgrade -y \
	&& apt-get install git pandoc pandoc-citeproc libcurl4-openssl-dev -y

RUN git clone https://github.com/jeroenooms/curl \
  && RD -e 'install.packages("curl", dependencies = TRUE)' \
  && RD CMD build curl --no-build-vignettes \
	&& RD CMD INSTALL curl_*.tar.gz --install-tests

ENV ASAN_OPTIONS=malloc_context_size=10:fast_unwind_on_malloc=false

RUN RD -e 'testthat::test_package("curl"); q("no");' || true

RUN RD CMD check curl_*.tar.gz
