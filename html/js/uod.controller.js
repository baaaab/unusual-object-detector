(function() {

	angular.module('uod').controller('uodController', [ '$http', '$timeout', '$q', uodController ]);

	function uodController($http, $timeout, $q) {
		var vm = this;

		vm.programData = {"sourceImageId":0, "matchImageId":0};
		vm.sourceHogs = {};
		vm.matchHogs = {};
		vm.unusualImageList = [];
		vm.scoreDistribution = [];
		vm.dygraph = new Dygraph(document.getElementById("scores_plot"), vm.scoreDistribution,
     {
      drawPoints: false,
      showRoller: false,
      labels: ['Images', 'Correlation']
    });

		vm.display = "images";
		vm.opacity = 100;

		refresh();
		$http.get("unusualImageList").then(getUnusualImageListComplete);

		function refresh() {
			$q.all([
				$http.get("programData").then(getProgramDataComplete),
				$http.get("hog/" + vm.programData.sourceImageId).then(getSourceHogComplete),
				$http.get("hog/" + vm.programData.matchImageId).then(getMatchHogComplete),
				$http.get("scoreDistribution").then(getScoreDistributionComplete),
			]).then(allRequestsComplete);
			
			function getProgramDataComplete(response) {
				vm.programData = response.data;
			}

			function getSourceHogComplete(response) {
				vm.sourceHogs = response.data.hog;
			}

			function getMatchHogComplete(response) {
				vm.matchHogs = response.data.hog;
			}
			
			function getScoreDistributionComplete(response) {
				vm.scoreDistribution = [];
				for(var i=0;i<response.data.scoreDistribution.length;i++)
				{
					vm.scoreDistribution.push([i, response.data.scoreDistribution[i]]);
				}
			}

			function allRequestsComplete(response) {
				draw();
				$timeout(refresh, 2000);
			}			
		}
		
		function getUnusualImageListComplete(response) {
			vm.unusualImageList = response.data.unusualImageList;
		}

		function draw() {
			if (vm.display == "hogs") {
				drawHogs("source_table", vm.programData.sourceImageId);
				drawHogs("match_table", vm.programData.matchImageId);
			} else if (vm.display == "images") {
				drawNone("source_table");
				drawNone("match_table");
			} else if (vm.display == "model") {
				drawModel("source_table");
				drawModel("match_table");
			}
			vm.dygraph.updateOptions( { 'file': vm.scoreDistribution } );
		}
		
		function drawNone(tableId) {
			var table = document.getElementById(tableId);
			while (table.firstChild) {
				table.removeChild(table.firstChild);
			}
		}

		function drawHogs(tableId, hogs) {
			var table = document.getElementById(tableId);
			while (table.firstChild) {
				table.removeChild(table.firstChild);
			}
			var tr = document.createElement("tr");
			var td = document.createElement("td");
			var img = document.createElement("img");
			img.src = "hogImageProvider/" + + ".jpg";

			td.appendChild(img);
			tr.appendChild(td);
			table.appendChild(tr);
		}

		function drawModel(tableId) {
			var table = document.getElementById(tableId);
			while (table.firstChild) {
				table.removeChild(table.firstChild);
			}

			var colours = [ "#ffffff", "#0000ff", "#00aaff", "#00aa00", "#ffff00", "#ff0000", "#ff00ff" ];
			var numLevels = vm.programData.model.length;

			for (var y = 0; y < vm.programData.maxNumCellsPerSide; y++) {
				var tr = document.createElement("tr");

				for (var x = 0; x < vm.programData.maxNumCellsPerSide; x++) {
					var td = document.createElement("td");

					var colour = "#000000";

					for (var level = 0; level <= numLevels; level++) {
						if (isHandledAtThisLevel(level, x >> (numLevels - level), y >> (numLevels - level)) && !isHandledAtHigherLevel(level, x >>(numLevels - level), y >> (numLevels - level))) {
							colour = colours[level];
							break;
						}
					}

					td.style.width = vm.programData.imageWidth / vm.programData.maxNumCellsPerSide + "px";
					td.style.height = vm.programData.imageHeight / vm.programData.maxNumCellsPerSide + "px";
					td.style.padding = "0px";
					td.style.margin = "0px";
					td.style.backgroundColor = colour;

					tr.appendChild(td);
				}
				table.appendChild(tr);
			}

			function isHandledAtHigherLevel(level, x, y) {
				//check this section in all earlier levels (must be 1)
				for (var i = 0; i < level && i < vm.programData.model.length; i++) {
					var edgeCellsThisLevel = 1 << i;

					if (!vm.programData.model[i][((y >> (level - i)) * edgeCellsThisLevel + (x >> (level - i)))]) {
						//this was handled at an earlier level
						return true;
					}
				}

				return false;
			}

			function isHandledAtThisLevel(level, x, y) {
				if (level == vm.programData.model.length) {
					return true;
				}
				var edgeCellsThisLevel = 1 << level;

				var isHandledAtThisLevel = vm.programData.model[level][(y * edgeCellsThisLevel + x)] == false;
				return isHandledAtThisLevel;
			}
		}
	}

})();