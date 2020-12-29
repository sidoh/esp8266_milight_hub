const fs = require('fs');
const { src, dest, series } = require('gulp');
const htmlmin = require('gulp-htmlmin');
const cleancss = require('gulp-clean-css');
const uglify = require('gulp-uglify');
const gzip = require('gulp-gzip');
const del = require('del');
const inline = require('gulp-inline');
const inlineImages = require('gulp-css-base64');
const favicon = require('gulp-base64-favicon');

const dataFolder = 'build/';

function clean() {
    return del([ dataFolder + '*']);
}

function buildfs_embeded() {
    var source = dataFolder + 'index.html.gz';
    var destination = dataFolder + 'index.html.gz.h';

    var wstream = fs.createWriteStream(destination);
    wstream.on('error', function (err) {
        console.log(err);
    });

    var data = fs.readFileSync(source);

    wstream.write('#define index_html_gz_len ' + data.length + '\n');
    wstream.write('static const char index_html_gz[] PROGMEM = {')

    for (i=0; i<data.length; i++) {
        wstream.write(data[i].toString());
        if (i<data.length-1) wstream.write(',');
    }

    wstream.write('};')
    wstream.end();

    return del();
}

function buildfs_inline() {
    return src('src/*.html')
        // .pipe(favicon())
        .pipe(inline({
            base: 'src/',
            js: uglify,
            css: [cleancss, inlineImages],
            disabledTypes: ['svg', 'img']
        }))
        .pipe(htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true
        }))
        .pipe(gzip())
        .pipe(dest(dataFolder));
}

exports.clean = clean;
exports.default = series(clean, buildfs_inline, buildfs_embeded);