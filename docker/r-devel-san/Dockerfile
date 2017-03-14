FROM rocker/r-devel-san

ENV UBSAN_OPTIONS print_stacktrace=1
ENV ASAN_OPTIONS malloc_context_size=10:fast_unwind_on_malloc=false

RUN apt-get -qq update \
	&& apt-get -qq dist-upgrade -y \
	&& apt-get -qq install git pandoc pandoc-citeproc libssl-dev libcurl4-openssl-dev -y \
  && RDscript -e 'install.packages("curl", dependencies = TRUE, quiet = TRUE)'

RUN git clone https://github.com/jeroen/curl \
  && RD CMD build curl --no-build-vignettes \
	&& RD CMD INSTALL curl_*.tar.gz --install-tests

RUN RDscript -e 'sessionInfo()'

RUN RDscript -e 'library(curl); testthat::test_dir("curl/tests/testthat")' || true

RUN RDscript -e 'library(curl); testthat::test_examples("curl/man")'|| true

RUN RD CMD check curl*.tar.gz
