// 搜索页面功能脚本
class SearchPage {
  constructor() {
    this.init();
  }

  init() {
    this.setupEventListeners();
    this.initializeSearchForm();
  }

  setupEventListeners() {
    // 购票按钮点击事件
    document.addEventListener('click', (e) => {
      if (e.target.classList.contains('btn-buy')) {
        this.handleBuyTicket(e.target);
      }
    });

    // 修改搜索按钮
    window.showModifySearch = () => {
      this.showModifySearchModal();
    };

    // 表单验证
    const forms = document.querySelectorAll('.needs-validation');
    forms.forEach(form => {
      form.addEventListener('submit', this.handleFormSubmit.bind(this));
    });
  }

  initializeSearchForm() {
    // 初始化日期选择器
    const dateInput = document.getElementById('date');
    if (dateInput && window.flatpickr) {
      flatpickr(dateInput, {
        locale: 'zh',
        dateFormat: 'm-d',
        minDate: 'today',
        maxDate: new Date().fp_incr(365)
      });
    }
  }

  handleBuyTicket(button) {
    const trainCard = button.closest('.train-card');
    if (!trainCard) return;

    const trainData = {
      id: trainCard.dataset.trainId,
      from: trainCard.dataset.from,
      to: trainCard.dataset.to,
      date: trainCard.dataset.date,
      price: trainCard.dataset.price,
      seats: trainCard.dataset.seats,
      startTime: trainCard.dataset.startTime,
      endTime: trainCard.dataset.endTime
    };

    this.showBuyModal(trainData);
  }

  showBuyModal(trainData) {
    const modal = document.getElementById('buyModal');
    if (!modal) return;

    // 填充模态框信息
    modal.querySelector('#buyTrainID').textContent = trainData.id;
    modal.querySelector('#buyRoute').textContent = `${trainData.from} → ${trainData.to}`;
    modal.querySelector('#buyDate').textContent = trainData.date;
    modal.querySelector('#buyTime').textContent = `${trainData.startTime} - ${trainData.endTime}`;
    modal.querySelector('#buyPrice').textContent = `¥${trainData.price}`;
    modal.querySelector('#buySeats').textContent = trainData.seats;

    // 设置隐藏字段
    modal.querySelector('#hiddenTrainID').value = trainData.id;
    modal.querySelector('#hiddenFrom').value = trainData.from;
    modal.querySelector('#hiddenTo').value = trainData.to;
    modal.querySelector('#hiddenDate').value = trainData.date;

    // 显示模态框
    const bsModal = new bootstrap.Modal(modal);
    bsModal.show();
  }

  showModifySearchModal() {
    // 创建修改搜索的模态框
    const modalHTML = `
      <div class="modal fade" id="modifySearchModal" tabindex="-1">
        <div class="modal-dialog">
          <div class="modal-content">
            <div class="modal-header">
              <h5 class="modal-title">修改搜索条件</h5>
              <button type="button" class="btn-close" data-bs-dismiss="modal"></button>
            </div>
            <form action="/search" method="GET">
              <div class="modal-body">
                <div class="row">
                  <div class="col-md-6 mb-3">
                    <label for="newFrom" class="form-label">出发站</label>
                    <input type="text" class="form-control" id="newFrom" name="from" required>
                  </div>
                  <div class="col-md-6 mb-3">
                    <label for="newTo" class="form-label">到达站</label>
                    <input type="text" class="form-control" id="newTo" name="to" required>
                  </div>
                </div>
                <div class="row">
                  <div class="col-md-6 mb-3">
                    <label for="newDate" class="form-label">出发日期</label>
                    <input type="text" class="form-control date-picker" id="newDate" name="date" required>
                  </div>
                  <div class="col-md-6 mb-3">
                    <label for="newSort" class="form-label">排序方式</label>
                    <select class="form-select" id="newSort" name="sort">
                      <option value="time">按时间排序</option>
                      <option value="cost">按价格排序</option>
                    </select>
                  </div>
                </div>
              </div>
              <div class="modal-footer">
                <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">取消</button>
                <button type="submit" class="btn btn-primary">
                  <i class="fas fa-search"></i> 重新搜索
                </button>
              </div>
            </form>
          </div>
        </div>
      </div>
    `;

    // 移除现有的模态框
    const existingModal = document.getElementById('modifySearchModal');
    if (existingModal) {
      existingModal.remove();
    }

    // 添加新模态框
    document.body.insertAdjacentHTML('beforeend', modalHTML);

    // 初始化日期选择器
    const newDateInput = document.getElementById('newDate');
    if (newDateInput && window.flatpickr) {
      flatpickr(newDateInput, {
        locale: 'zh',
        dateFormat: 'm-d',
        minDate: 'today',
        maxDate: new Date().fp_incr(365)
      });
    }

    // 显示模态框
    const modal = new bootstrap.Modal(document.getElementById('modifySearchModal'));
    modal.show();
  }

  handleFormSubmit(event) {
    const form = event.target;
    
    if (!form.checkValidity()) {
      event.preventDefault();
      event.stopPropagation();
    }
    
    form.classList.add('was-validated');

    // 如果是购票表单，添加特殊处理
    if (form.id === 'buyForm') {
      event.preventDefault();
      this.handleBuyFormSubmit(form);
    }
  }

  async handleBuyFormSubmit(form) {
    const formData = new FormData(form);
    const data = Object.fromEntries(formData.entries());

    // 显示加载状态
    const submitBtn = form.querySelector('button[type="submit"]');
    const originalText = submitBtn.innerHTML;
    submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> 处理中...';
    submitBtn.disabled = true;

    try {
      const response = await fetch('/api/order/buy', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      });

      const result = await response.json();

      if (result.success) {
        // 购票成功
        this.showSuccessMessage('购票成功！', result);
        
        // 关闭模态框
        const modal = bootstrap.Modal.getInstance(document.getElementById('buyModal'));
        modal.hide();
        
        // 可以选择跳转到订单页面
        setTimeout(() => {
          if (confirm('是否查看订单详情？')) {
            window.location.href = '/orders';
          }
        }, 1500);
      } else {
        // 购票失败
        this.showErrorMessage(result.message || '购票失败');
      }
    } catch (error) {
      console.error('购票错误:', error);
      this.showErrorMessage('网络错误，请重试');
    } finally {
      // 恢复按钮状态
      submitBtn.innerHTML = originalText;
      submitBtn.disabled = false;
    }
  }

  showSuccessMessage(message, details = null) {
    const alertHTML = `
      <div class="alert alert-success alert-dismissible fade show" role="alert">
        <i class="fas fa-check-circle me-2"></i>
        <strong>成功！</strong> ${message}
        ${details && details.totalPrice ? `<br><small>总金额: ¥${details.totalPrice}</small>` : ''}
        ${details && details.queued ? `<br><small class="text-warning">已加入候补队列</small>` : ''}
        <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
      </div>
    `;
    
    this.showAlert(alertHTML);
  }

  showErrorMessage(message) {
    const alertHTML = `
      <div class="alert alert-danger alert-dismissible fade show" role="alert">
        <i class="fas fa-exclamation-circle me-2"></i>
        <strong>错误！</strong> ${message}
        <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
      </div>
    `;
    
    this.showAlert(alertHTML);
  }

  showAlert(alertHTML) {
    const container = document.querySelector('.container');
    const firstCard = container.querySelector('.card');
    
    // 移除现有的提示
    const existingAlerts = container.querySelectorAll('.alert');
    existingAlerts.forEach(alert => alert.remove());
    
    // 插入新提示
    firstCard.insertAdjacentHTML('beforebegin', alertHTML);
    
    // 滚动到顶部
    window.scrollTo({ top: 0, behavior: 'smooth' });
  }
}

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', () => {
  window.searchPage = new SearchPage();
});

// 全局函数，供模板调用
window.buyTicket = (trainId, from, to, date, price, seats, startTime, endTime) => {
  if (window.searchPage) {
    window.searchPage.handleBuyTicket({
      closest: () => ({
        dataset: {
          trainId, from, to, date, price, seats, startTime, endTime
        }
      })
    });
  }
};
