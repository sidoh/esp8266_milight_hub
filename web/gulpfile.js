const fs = require('fs');
const gulp = require('gulp');
const htmlmin = require('gulp-htmlmin');
const cleancss = require('gulp-clean-css');
const uglify = require('gulp-uglify');
const gzip = require('gulp-gzip');
const del = require('del');
const inline = require('gulp-inline');
const inlineImages = require('gulp-css-base64');
const favicon = require('gulp-base64-favicon');

const dataFolder = 'build/';

gulp.task('clean', function() {
    del([ dataFolder + '*']);
    return true;
});

gulp.task('buildfs_embeded', ['buildfs_inline'], function() {

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

    del();

});

gulp.task('buildfs_inline', ['clean'], function() {
    return gulp.src('src/*.html')
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
        .pipe(gulp.dest(dataFolder));
})

gulp.task('default', ['buildfs_embeded']);
