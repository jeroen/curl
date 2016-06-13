FROM rocker/r-devel-san

RUN apt-get -qq update \
	&& apt-get -qq dist-upgrade -y \
	&& apt-get -qq install git pandoc pandoc-citeproc libssl-dev libcurl4-openssl-dev -y

RUN RD -e 'install.packages("devtools", quiet = T)' \
  && RD -e 'devtools::install_github("jeroenooms/jsonlite", quiet = T)'

RUN git clone https://github.com/jeroenooms/curl \
  && RD -e 'devtools::install("curl", dependencies = TRUE, quiet = T)' \
  && RD CMD build curl --no-build-vignettes \
	&& RD CMD INSTALL curl_*.tar.gz --install-tests

ENV ASAN_OPTIONS=malloc_context_size=10:fast_unwind_on_malloc=false

RUN RD -e 'testthat::test_package("curl"); q("no");' || true

RUN RD CMD check curl_*.tar.gz
