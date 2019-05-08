(function() {

	angular.module('uod').controller('uodController', [ '$http', '$timeout', '$q', uodController ]);

	function uodController($http, $timeout, $q) {
		var vm = this;

		vm.programData = {"sourceImageId":0, "matchImageId":0};
		vm.sourceHogs = {};
		vm.matchHogs = {};

		vm.display = "images";
		vm.opacity = 100;

		refresh();

		function refresh() {
			$q.all([
				$http.get("programData").then(getProgramDataComplete),
				$http.get("hog/" + vm.programData.sourceImageId).then(getSourceHogComplete),
				$http.get("hog/" + vm.programData.matchImageId).then(getMatchHogComplete),
			]).then(allRequestsComplete);

			function allRequestsComplete(response) {
				draw();
				$timeout(refresh, 2000);
			}
		}

		function getProgramDataComplete(response) {
			vm.programData = response.data;
		}

		function getSourceHogComplete(response) {
			vm.sourceHogs = response.data.hog;
		}

		function getMatchHogComplete(response) {
			vm.matchHogs = response.data.hog;
		}

		function draw() {
			if (vm.display == "hogs") {
				drawHogs("source_table", vm.sourceHogs);
				drawHogs("match_table", vm.matchHogs);
			} else if (vm.display == "images") {
				drawNone("source_table");
				drawNone("match_table");
			} else if (vm.display == "model") {
				drawModel("source_table");
				drawModel("match_table");
			}
		}
		
		function drawNone(tableId) {
			/*var table = document.getElementById(tableId);
			while (table.firstChild) {
				table.removeChild(table.firstChild);
			}
			
			var tr = document.createElement("tr");
			var td = document.createElement("td");
			
			td.style.height = vm.programData.imageHeight = "px";
			td.style.width = vm.programData.imageWidth = "px";
			
			tr.appendChild(td);
			table.appendChild(tr);*/
		}

		function drawHogs(tableId, hogs) {
			var table = document.getElementById(tableId);
			while (table.firstChild) {
				table.removeChild(table.firstChild);
			}

			var scalingFactor = 0;
			for (var i = 0; i < hogs.length; i++) {
				scalingFactor = Math.max(scalingFactor, hogs[i]);
			}

			for (var y = 0; y < vm.programData.maxNumCellsPerSide; y++) {
				var tr = document.createElement("tr");

				for (var x = 0; x < vm.programData.maxNumCellsPerSide; x++) {
					var td = document.createElement("td");

					td.style.width = vm.programData.imageWidth / vm.programData.maxNumCellsPerSide + "px";
					td.style.height = vm.programData.imageHeight / vm.programData.maxNumCellsPerSide + "px";
					td.style.padding = "0px";
					td.style.margin = "0px";
					td.style.zIndex = x*1000 + y;
					
					var pos = document.createElement("div");
					pos.style.position = "relative";
					pos.style.top = "0px";
					pos.style.width = vm.programData.imageWidth / vm.programData.maxNumCellsPerSide + "px";
					pos.style.height = vm.programData.imageHeight / vm.programData.maxNumCellsPerSide + "px";
					pos.style.textAlign = "top";
					pos.style.padding = "0px";
					pos.style.margin = "0px";
					

					var hogOffset = (y * vm.programData.maxNumCellsPerSide * 8) + (x * 8);

					for (var i = 0; i < 8; i++) {
						var div = document.createElement("div");

						var height = (hogs[hogOffset + i] / scalingFactor) * 8;

						div.style.position = "absolute";
						div.style.top = "0px";
						div.style.left = i +"px";
						div.style.height = height + "px";
						div.style.width = "1px";
						div.style.padding = "0px";
						div.style.margin = "0px";
						div.style.backgroundColor = "#ff0000";

						pos.appendChild(div);
					}
					td.appendChild(pos);
					tr.appendChild(td);
				}
				table.appendChild(tr);
			}
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