jQuery(document).ready(function () {

    //Global variables to store data needed between different server requests
    var userString = "";
    var selectedShapeName = "";
    var selectedShapeIndex = 0;

    //Function to setup and format the view panel based on the currently selected SVG
    function vpFormat(displaySVG, file) {
        $('#svg-view-panel').empty();
        $('#vp-image').empty();
        $('#vp-image').append('<img class="table-picture" src="' + file + '">');

        var temp1 = '<div class="col lead font-weight-bold border-right border-dark">Title</div>' + '<div class="col lead font-weight-bold border-right border-dark">Description</div>';
        $('#svg-view-panel').append('<div class="row border-top border-dark">' + temp1 + '</div>');

        var temp2 = '<div class="col border-right border-dark">' + displaySVG.title + '</div>' + '<div class="col border-right border-dark">' + displaySVG.desc + '</div>';
        $('#svg-view-panel').append('<div class="row border-top border-dark">' + temp2 + '</div>');

        var temp3 = '<div class="col lead font-weight-bold border-right border-dark">Component</div>' + '<div class="col lead font-weight-bold border-right border-dark">Summary</div>' + '<div class="col lead font-weight-bold border-right border-dark">Other Attributes</div>';
        $('#svg-view-panel').append('<div class="row border-top border-dark">' + temp3 + '</div>');

        for (let i = 0; i < displaySVG.rects.length; i++) {
            var temp = i + 1;
            var temp4 = '<div class="col border-right border-dark">Rectangle ' + temp + '</div>';
            if (!(displaySVG.rects[i].units)) {
                temp4 += '<div class="col border-right border-dark">x = ' + displaySVG.rects[i].x + ' y = ' + displaySVG.rects[i].y + ' Width = ' + displaySVG.rects[i].w + ' Height = ' + displaySVG.rects[i].h + '</div>';
            }
            else {
                temp4 += '<div class="col border-right border-dark">x = ' + displaySVG.rects[i].x + ' y = ' + displaySVG.rects[i].y + ' Width = ' + displaySVG.rects[i].w + ' Height = ' + displaySVG.rects[i].h + ' Units = ' + displaySVG.rects[i].units + '</div>';
            }
            temp4 += '<div class="col border-right border-dark">' + displaySVG.rects[i].numAttr + '</div>';

            $('#svg-view-panel').append('<div class="row border-top border-dark">' + temp4 + '</div>');
        }

        for (let i = 0; i < displaySVG.circles.length; i++) {
            var temp = i + 1;
            var temp5 = '<div class="col border-right border-dark">Circle ' + temp + '</div>';
            if (!(displaySVG.circles[i].units)) {
                temp5 += '<div class="col border-right border-dark">x = ' + displaySVG.circles[i].cx + ' y = ' + displaySVG.circles[i].cy + ' r = ' + displaySVG.circles[i].r + '</div>';
            }
            else {
                temp5 += '<div class="col border-right border-dark">x = ' + displaySVG.circles[i].cx + ' y = ' + displaySVG.circles[i].cy + ' r = ' + displaySVG.circles[i].r + ' units = ' + displaySVG.circles[i].units + '</div>';
            }
            temp5 += '<div class="col border-right border-dark">' + displaySVG.circles[i].numAttr + '</div>';

            $('#svg-view-panel').append('<div class="row border-top border-dark">' + temp5 + '</div>');
        }

        for (let i = 0; i < displaySVG.paths.length; i++) {
            var temp = i + 1;
            console.log(displaySVG.paths.length);
            var temp6 = '<div class="col border-right border-dark">Path ' + temp + '</div>' + '<div class="col border-right border-dark">x = ' + displaySVG.paths[i].d + '</div>' + '<div class="col border-right border-dark">' + displaySVG.paths[i].numAttr + '</div>';
            $('#svg-view-panel').append('<div class="row border-top border-dark">' + temp6 + '</div>');
        }

        for (let i = 0; i < displaySVG.groups.length; i++) {
            var temp = i + 1;
            var temp7 = '<div class="col border-right border-dark">Group ' + temp + '</div>' + '<div class="col border-right border-dark">' + displaySVG.groups[i].children + ' child elements</div>' + '<div class="col border-right border-dark">' + displaySVG.groups[i].numAttr + '</div>';
            $('#svg-view-panel').append('<div class="row border-top border-dark">' + temp7 + '</div>');
        }
    }

    //On page load, setup Log Panel and File select menu
    jQuery.ajax({
        url: '/getUploadFiles',
        type: 'GET',
        dataType: 'json',
        success: function (data) {
            if (data.img.length == 0) {
                $('#file-log-table').append('<div class="lead">No Files</div>');
            }

            else {
                for (let i = 0; i < data.img.length; i++) {
                    $('#svg-menu').append('<option>' + data.img[i].name + '</option>');

                    var mainTemp = '<th><a download="' + data.img[i].name + '" href="' + data.img[i].name + '" title="Click to Download"><img class="picture-limit" src="' + data.img[i].name + '"/></a></th>';
                    mainTemp += '<th><a href="' + data.img[i].name + '" download>' + data.img[i].name + '</a></th>';
                    mainTemp += '<td>' + data.img[i].size + ' kb</td>';
                    mainTemp += '<td>' + data.img[i].numRect + '</td>';
                    mainTemp += '<td>' + data.img[i].numCirc + '</td>';
                    mainTemp += '<td>' + data.img[i].numPaths + '</td>';
                    mainTemp += '<td>' + data.img[i].numGroups + '</td>';
                    $('#file-log-table').append('<tbody><tr>' + mainTemp + '</tr></tbody>');
                }
            }
        },
        error: function (e) {
            console.log(e);
        }
    });

    $('#svg-upload').submit(function (userFile) {
        userFile.preventDefault();

        var elementId = $('#svg-upload')[0];
        var formData = new FormData(elementId);

        $.ajax({
            url: '/validateUpload',
            type: 'POST',
            data: formData,
            contentType: false,
            processData: false,
            success: function () {
                alert('File successfully uploaded');
                location.reload();
            },
            error: function (e) {
                alert('Error occured');
                console.log(e);
            }
        });
    });

    $('#make-new-svg').submit(function (userFile) {
        userFile.preventDefault();

        var newName = $('#new-filename').val();
        var newDescription = $('#new-description').val();
        var newTitle = $('#new-title').val();

        $.ajax({
            url: '/userCreatedSVG',
            type: 'POST',
            data: {
                filename: newName,
                desc: newDescription,
                title: newTitle,
            },
            success: function () {
                location.reload();
            },
            error: function (e) {
                alert('Invalid input sizes');
                console.log(e);
            },
        });
    });

    $('#svg-menu').change(function () {
        var temp = "";

        $("select option:selected").each(function () {
            temp += $(this).text();
        });

        userString = temp;

        $.ajax({
            url: '/userSelectedSVG',
            type: 'GET',
            data: { file: userString },
            dataType: 'json',
            success: function (svgData) {
                $('#all-attribute-list').empty();
                vpFormat(svgData.panel, userString);
            },
            error: function (e) {
                alert('Error occured');
                console.log(e);
            },
        });
    });

    $('#attribute-display-submit').click(function (call) {
        call.preventDefault();

        var tempOne = $('#select-shape').val();
        var tempTwo = $('#select-num').val();

        selectedShapeIndex = tempTwo;
        selectedShapeName = tempOne;

        $.ajax({
            url: '/getAttributeFromShape',
            type: 'GET',
            data: {
                shape: selectedShapeName,
                number: selectedShapeIndex,
                file: userString,
            },
            dataType: 'json',
            success: function (shape) {
                $('#all-attribute-list').empty();
                var temp8 = '<div class="col lead font-weight-bold border-right border-dark">Name</div>' + '<div class="col lead font-weight-bold border-right border-dark">Value</div>';
                $('#all-attribute-list').append('<div class="row border-top border-dark">' + temp8 + '</div>');
                for (let i = 0; i < shape.data.otherAttributes.length; i++) {
                    var temp8 = '<div class="col border-right border-dark">' + shape.data.otherAttributes[i].name + '</div>' + '<div class="col border-right border-dark">' + shape.data.otherAttributes[i].value + '</div>';
                    $('#all-attribute-list').append('<div class="row border-top border-dark">' + temp8 + '</div>');
                }
            },
            error: function (e) {
                alert('error');
                console.log(e);
            },
        });
    });

    $('#attribute-edit-submit').click(function (call) {
        var attributeName = $('#select-attribute-type').val();
        var attributeValue = $('#select-attribute-val').val();

        if (selectedShapeName != "" && selectedShapeIndex != 0) {
            $.ajax({
                url: '/setAttributeInShape',
                type: 'POST',
                data: {
                    shape: selectedShapeName,
                    number: selectedShapeIndex,
                    attrName: attributeName,
                    attrVal: attributeValue,
                    file: userString,
                },
                success: function () {
                    alert('Attribute successfully edited');
                    location.reload();
                },
                error: function (e) {
                    alert('Invalid attribute');
                    console.log(e);
                },
            });
        }
        else {
            alert('Make sure to select a shape to change');
        }
    });

    $('#title-change-form').submit(function (call) {
        call.preventDefault();

        var titleName = $('#title-text-box').val();

        $.ajax({
            url: '/changeTitle',
            type: 'POST',
            data: {
                title: titleName,
                file: userString,
            },
            success: function () {
                alert('Title successfully changed');
                location.reload();
            },
            error: function (e) {
                alert('Invalid title size');
                console.log(e);
            },
        });
    });

    $('#description-change-form').submit(function (call) {
        call.preventDefault();

        var descTotal = $('#description-text-box').val();

        $.ajax({
            url: '/changeDesc',
            type: 'POST',
            data: {
                desc: descTotal,
                file: userString,
            },
            success: function () {
                alert('Description successfully changed');
                location.reload();
            },
            error: function (e) {
                alert('Invalid description size');
                console.log(e);
            },
        });
    });

    $('#new-rect').submit(function (userRect) {
        userRect.preventDefault();

        var userX = $('#x-input').val();
        var userY = $('#y-input').val();
        var userWidth = $('#width-input').val();
        var userHeight = $('#height-input').val();
        var userUnit = $('#rect-unit-input').val();

        $.ajax({
            url: '/addRect',
            type: 'POST',
            data: {
                file: userString,
                x: userX,
                y: userY,
                width: userWidth,
                height: userHeight,
                units: userUnit
            },
            success: function () {
                location.reload();
            },
            error: function (e) {
                alert('Invalid rectangle parameters');
                console.log(e);
            },
        });
    });

    $('#new-circle').submit(function (userCirc) {
        userCirc.preventDefault();

        var userX = $('#cx-input').val();
        var userY = $('#cy-input').val();
        var userR = $('#r-input').val();
        var userUnit = $('#circle-unit-input').val();

        $.ajax({
            url: '/addCircle',
            type: 'POST',
            data: {
                file: userString,
                x: userX,
                y: userY,
                r: userR,
                units: userUnit
            },
            success: function () {
                location.reload();
            },
            error: function (error) {
                alert('Invalid circle parameters');
                console.log(e);
            },
        });
    });

    $('#rect-scale-form').submit(function (e) {
        e.preventDefault();

        var userScale = $('#rect-scale-size').val();

        $.ajax({
            url: '/scaleRects',
            type: 'POST',
            data: {
                file: userString,
                scale: userScale,
            },
            success: function () {
                location.reload();
            },
            error: function (error) {
                alert('Scale is invalid');
                console.log(e);
            },
        });
    });

    $('#circle-scale-form').submit(function (e) {
        e.preventDefault();

        var userScale = $('#circle-scale-size').val();

        $.ajax({
            url: '/scaleCircles',
            type: 'POST',
            data: {
                file: userString,
                scale: userScale,
            },
            success: function () {
                location.reload();
            },
            error: function (error) {
                alert('Scale is invalid');
                console.log(e);
            },
        });
    });
});
